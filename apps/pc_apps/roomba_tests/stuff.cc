/**
 * @file helpers.cpp
 * @date 8 Mar 2011
 * @author Roland Hieber <rohieb@rohieb.name>
 */

#include "stuff.h"
#include <fstream>

using namespace std;

/**
 * Global variables, so we do not have to carry them around each time
 */
OsModel::Os g_os;
OsModel::Timer::self_t g_timer;
Roomba g_roomba;
RoombaUart g_roomba_uart(g_os);
ControlledMotion g_ctrl_motion;
char * g_roomba_id;
char * g_ground_type;
char g_log_file[64];
char g_uart[] = "/dev/ttyUSB0";
SensorData g_sensor_data;
DataAvailable g_data_available;

/**
 * return battery status as QString
 */
QString charge_text() {
  return QString("Battery: %1%\nPress Cancel to exit.\n\n").arg(int(float(
    g_sensor_data.charge) / float(g_sensor_data.capacity) * 100.0));
}

/**
 * return log text for global values
 */
QString global_log_text() {
  return QString("svn=%1 roomba_id=%2 ground_type=%3 diff_left_ticks=%4 "
    "diff_right_ticks=%5 raw_ticks_left=%6 raw_ticks_right=%7 batt_charge=%8 "
    "batt_capacity=%9 batt_voltage=%10 batt_current=%11").arg(SVNREVISION).arg(
    g_roomba_id).arg(g_ground_type).arg(g_sensor_data.diff_left_ticks).arg(
    g_sensor_data.diff_right_ticks).arg(g_sensor_data.raw_left_ticks).arg(
    g_sensor_data.raw_right_ticks).arg(g_sensor_data.charge).arg(
    g_sensor_data.capacity).arg(g_sensor_data.voltage).arg(
    g_sensor_data.current);
}

/**
 * Log text to file
 */
void log(QString text) {
  std::ofstream log(g_log_file, ios::app);
  log << text.toUtf8().constData() << std::endl;
  log.close();
}

/**
 * Returns the difference between two unsigned short values. The calculated
 * value is always smaller or equal to 0x8000.
 * This is useful if you have an overflowing counter and you want to determine
 * when you have to "wrap over" the value.
 */
int nearest_diff(unsigned short last, unsigned short current) {
  int d = current - last;
  if(d < -0x8000) { // overflow in positive direction
    d = (0x10000 - last + current);
  }
  if(d >= 0x8000) { // overflow in negative direction
    d = -(0x10000 - current + last);
  }
}

/**
 * SensorData constructor
 */
SensorData::SensorData() :
  capacity(0), charge(0), charging(0), current(0), temperature(0), voltage(0),
    diff_left_ticks(0), diff_right_ticks(0) {
}

/**
 * Initialisation. Calling this function is optional, but strongly encouraged
 * if you want to get right results.
 * @param init_ticks_left Initial ticks of left wheel used to calculate the
 *  difference
 * @param init_ticks_left Initial ticks of right wheel used to calculate the
 *  difference
 */
void DataAvailable::init(int init_ticks_left, int init_ticks_right) {
  latest_ticks_left_ = init_ticks_left;
  latest_ticks_right_ = init_ticks_right;
}

/**
 * Callback for Roomba<...>::register_state_callback()
 * @param state Callback parameter that describes the state ot the data
 */
void DataAvailable::cb(int state) {
  if(state != Roomba::DATA_AVAILABLE) {
    return;
  }
  g_sensor_data.angle = g_roomba().angle;
  g_sensor_data.capacity = g_roomba().capacity;
  g_sensor_data.charge = g_roomba().charge;
  g_sensor_data.charging = g_roomba().charging;
  g_sensor_data.current = g_roomba().current;
  g_sensor_data.distance = g_roomba().distance;
  g_sensor_data.voltage = g_roomba().voltage;
  g_sensor_data.raw_left_ticks = g_roomba().left_encoder_counts;
  g_sensor_data.raw_right_ticks = g_roomba().right_encoder_counts;
  g_sensor_data.diff_left_ticks += nearest_diff(latest_ticks_left_,
    g_roomba().left_encoder_counts);
  latest_ticks_left_ = g_roomba().left_encoder_counts;
  g_sensor_data.diff_right_ticks += nearest_diff(latest_ticks_right_,
    g_roomba().right_encoder_counts);
  latest_ticks_right_ = g_roomba().right_encoder_counts;
}
