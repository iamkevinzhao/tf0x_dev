#include "driver.h"
#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QElapsedTimer>
#include "static_unique_ptr_cast.h"

std::unordered_map<char, Lingual> Driver::kEchoStatusIDMap{
  {0x44, {"Communication Protocol", "通信协议"}}
};

Driver::Driver() {
}

bool Driver::Open() {
  LoadAllParsers(receive_parsers_);
  stop_signal_ = false;
  work_thead_ = std::thread(&Driver::WorkThread, this);
  return true;
}

bool Driver::Close() {
  stop_signal_ = true;
  if (work_thead_.joinable()) {
    work_thead_.join();
  }
  return true;
}

bool Driver::LastMeasure(MeasureBasic &measure) {
  latest_measure_mutex_.lock();
  auto ptr =
      static_unique_ptr_cast<MeasureBasic>(std::move(latest_measure_.data));
  latest_measure_mutex_.unlock();
  if (ptr) {
    measure = *ptr;
    return true;
  } else {
    return false;
  }
}

void Driver::WorkThread() {
  serial_port_.reset(new QSerialPort);
  serial_port_->setBaudRate(115200);
  auto ports = QSerialPortInfo::availablePorts();
  if (ports.empty()) {
    return;
  }
  serial_port_->setPortName(ports.begin()->portName());
  if (!serial_port_->open(QIODevice::ReadWrite)) {
    return;
  }

  while (!stop_signal_) {
    HandleIncomingCommandInWorkThread();
    if (serial_port_->waitForReadyRead(100)) {
      buffer_ += serial_port_->readAll();
      ProcessBufferInWorkThread(buffer_);
    }
  }
  serial_port_->close();
}

void Driver::HandleIncomingCommandInWorkThread() {
  command_queue_mutex_.lock();
  auto queue = command_queue_;
  while (!command_queue_.empty()) {
    command_queue_.pop();
  }
  command_queue_mutex_.unlock();

  while (!queue.empty()) {
    auto command = queue.front();
    command();
    queue.pop();
  }
}

void Driver::ProcessBufferInWorkThread(QByteArray &buffer) {
  Message parsed;
  while (true) {
    int from = 0, to = 0;
    int parsed_cnt = 0;
    for (auto& parser : receive_parsers_) {
      if (parser(buffer, parsed, from, to)) {
        ++parsed_cnt;
        buffer = buffer.remove(0, to + 1);
        if (parsed.type == MessageType::measure) {
          latest_measure_mutex_.lock();
          latest_measure_ = std::move(parsed);
          latest_measure_mutex_.unlock();
        } else {
          receive_messages_mutex_.lock();
          receive_messages_.emplace_back(std::move(parsed));
          receive_messages_mutex_.unlock();
        }
      }
    }
    if (parsed_cnt == 0) {
      break;
    }
  }
}

bool Driver::SendMessage(const QByteArray &msg) {
  if (!serial_port_) {
    return false;
  }
  if (!serial_port_->isOpen()) {
    return false;
  }
  serial_port_->write(msg);
  return true;
}
