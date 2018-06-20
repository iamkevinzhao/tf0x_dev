#ifndef TF03_DRIVER_H
#define TF03_DRIVER_H

#include "tf0x_driver.h"
#include "export.h"
#include <memory>
#include "abstract_serial_port.h"
#include <vector>
#include <QElapsedTimer>

namespace tf03_driver {
struct Measurement {
  Measurement()
    : dist1(0),
      dist2(0),
      dist3(0),
      raw_dist1(0),
      raw_dist2(0),
      raw_dist3(0),
      apd(0),
      volt(0),
      temp(0.0f),
      ts(0) {}
  uint16_t dist1;
  uint16_t dist2;
  uint16_t dist3;
  uint16_t raw_dist1;
  uint16_t raw_dist2;
  uint16_t raw_dist3;
  unsigned char apd;
  uint16_t volt;
  float temp; // Celsius
  inline uint16_t DistanceMax() const {
    return 18000;
  }
  uint16_t algorithm;
  long ts;
};

struct CartMeasurement {
  CartMeasurement() : id(0), pos(0) {}
  int id;
  unsigned short pos;
  Measurement measurement;
};

enum class OutputFormat {
  ascii = 0x00,
  binary = 0x01,
  pix = 0x02,
  primitive = 0x03
};

enum class ProtocolType {
  release = 0x01,
  develop = 0x02
};

enum class TransType {
  serial = 0x01,
  can = 0x02
};

struct SetAPDEcho {
  bool success;
};

class API Driver: public tf0x_driver::Driver
{
 public:
  bool Initialize();
  void SetSerialPort(std::shared_ptr<tf0x_driver::AbstractSerialPort> port);
  bool ReadDistance(double &distance) override;
//  bool ReadMeasurement(Measurement& measurement);
//  bool ReadMeasurement(Measurement &measurement, std::string& buffer);
  std::vector<Measurement> ReadMeasurements(std::string& buffer);

  // Operations
  std::string GetVersion();
  bool ResetDevice();
  bool SetFrequency(const uint16_t& frequency);
  bool MeasureOnce(Measurement& Measurement);
  bool SetOutputFormat(const OutputFormat& format);
  bool SetBaudRate(const int& baud_rate);
  bool Enable(const bool& enable);
  bool EnableCheckSum(const bool& enable);
  bool RestoreFactory();
  bool SaveSettings();
  bool SetAPD(const uint8_t& value);
  bool SetVdbs(const uint16_t& value);
  bool SetTableCorrA(const double& value);
  bool SetTableCorrB(const int16_t& value);
  bool SetProtocolType(const ProtocolType& type);
  bool SetTransType(const TransType& type);
  bool SetSplineBreaks(const std::vector<int16_t>& array);
  bool SetSplineCoefs(const std::vector<std::vector<int16_t>>& matrix);
  std::vector<SetAPDEcho> set_apd_echo;
 private:
  bool DetectAndHandleEcho();
  bool IsValidEchoBuffer(const std::string& buffer);
  static bool IsValidBuffer(const std::string& buffer);
  Measurement ParseBuffer(const std::string& buffer);
  std::shared_ptr<tf0x_driver::AbstractSerialPort> serial_port_;
  static std::string Head();
  static std::string AppendCheckSum(const std::string& buffer);
  QElapsedTimer timer_;
  std::string buffer_;
};
} // namespace tf03_driver

#endif // TF03_DRIVER_H
