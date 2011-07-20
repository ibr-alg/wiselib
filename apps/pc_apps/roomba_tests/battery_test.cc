/**
 * @file battery_test.cc
 * @date 1 Dec 2010
 * @author Roland Hieber <rohieb@rohieb.name>
 *
 * Application that writes the Roomba's battery status to stdout repeatedly.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <iostream>
#include <stdint.h>
#include <external_interface/pc/pc_os_model.h>
#include <external_interface/pc/pc_com_uart.h>
#include <external_interface/pc/pc_timer.h>
#include <intermediate/robot/roomba/roomba.h>
#include <intermediate/robot/controlled_motion.h>

using namespace std;

// UART port on which we communicate with the Roomba
char g_uart[] = "/dev/ttyUSB0";

typedef wiselib::PCOsModel OsModel;
typedef wiselib::StandaloneMath<OsModel> Math;
typedef wiselib::PCComUartModel<OsModel, g_uart> RoombaUart;
typedef wiselib::RoombaModel<OsModel, RoombaUart> Roomba;
typedef wiselib::ControlledMotion<OsModel, Roomba> ControlledMotion;

/**
 * Global objects we need
 */
OsModel::Os g_os;
OsModel::Timer::self_t g_timer;
Roomba g_roomba;
RoombaUart g_roomba_uart(g_os);
ControlledMotion g_ctrl_motion;

/**
 * Sensor data we need, filled in callback
 */
struct SensorData {
  uint16_t capacity, charge;
  uint8_t charging;
  int16_t current;
  int8_t temperature;
  uint16_t voltage;
} g_sensor_data;

/**
 * Callback that fills the sensor data when data is available
 */
struct DataAvailable {
  void cb(int foo) {
    g_sensor_data.capacity = g_roomba().capacity;
    g_sensor_data.charge = g_roomba().charge;
    g_sensor_data.charging = g_roomba().charging;
    g_sensor_data.current = g_roomba().current;
    g_sensor_data.voltage = g_roomba().voltage;
  }
} g_data_available;

/**
 * main function
 */
int main(int argc, char ** argv) {

  // init stuff
  g_roomba_uart.set_baudrate(19200);
  g_roomba_uart.enable_serial_comm();
  g_roomba.init(g_roomba_uart, g_timer, Roomba::BATTERY_AND_TEMPERATURE);

  cerr << "Got roomba at " << g_roomba_uart.address() << endl;

  g_roomba.register_state_callback<DataAvailable, &DataAvailable::cb> (
    &g_data_available);
  g_roomba.notify_state_receivers(Roomba::DATA_AVAILABLE);

  while(true) {
    cout << "batt_charge=" << g_sensor_data.charge << "\t batt_capacity="
      << g_sensor_data.capacity << "\t batt_voltage=" << g_sensor_data.voltage
      << "\t batt_current=" << g_sensor_data.current << endl;
    sleep(1000);
  }
}
