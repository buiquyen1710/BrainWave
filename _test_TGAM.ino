#include <BluetoothSerial.h>
#include <HardwareSerial.h>

BluetoothSerial BTSerial;
HardwareSerial TGAMSerial(1); // UART1 cho TGAM

void setup() {
  Serial.begin(115200);
  TGAMSerial.begin(57600, SERIAL_8N1, 16, 17); // RX, TX
  BTSerial.begin("TGAM-ESP32"); // Tên Bluetooth sẽ hiển thị trên điện thoại
  Serial.println("Bluetooth đã sẵn sàng. Tên: TGAM-ESP32");
}

void loop() {
  while (TGAMSerial.available()) {
    byte b = TGAMSerial.read();
    BTSerial.write(b);  // Gửi dữ liệu sang điện thoại qua Bluetooth
    Serial.write(b);    // In ra Serial để debug
  }
}
