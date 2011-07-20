
/**
 * @file stuff.h
 * @date 8 Mar 2011
 * @author Roland Hieber <rohieb@rohieb.name>
 */

#ifndef STUFF_H_
#define STUFF_H_

#include "svnrevision.h"
#include <external_interface/pc/pc_os_model.h>
#include <external_interface/pc/pc_com_uart.h>
#include <external_interface/pc/pc_timer.h>
#include <intermediate/robot/roomba/roomba.h>
#include <intermediate/robot/controlled_motion.h>
#include <QString>
#include <QInputDialog>

// Hack for different Qt versions
#if QT_VERSION < 0x040500
#define get_int QInputDialog::getInteger
#else // QT_VERSION > 0x040500
#define get_int QInputDialog::getInt
#endif // QT_VERSION

// UART port on which we communicate with the Roomba
extern char g_uart[];

/**
 * Basic types
 */
typedef wiselib::PCOsModel OsModel;
typedef OsModel::Timer::self_t Timer;
typedef wiselib::StandaloneMath<OsModel> Math;
typedef wiselib::PCComUartModel<OsModel, g_uart> RoombaUart;
typedef wiselib::RoombaModel<OsModel, RoombaUart> Roomba;
typedef wiselib::ControlledMotion<OsModel, Roomba> ControlledMotion;

/**
 * Global variables, so we do not have to carry them around each time
 */
extern OsModel::Os g_os;
extern Timer g_timer;
extern Roomba g_roomba;
extern RoombaUart g_roomba_uart;
extern ControlledMotion g_ctrl_motion;
extern char * g_roomba_id;
extern char * g_ground_type;
extern char g_log_file[];

/**
 * Sensor data we need, filled in callback
 */
struct SensorData {
  SensorData();

  uint16_t capacity, charge;
  uint8_t charging;
  int16_t current;
  int16_t angle, distance;
  int8_t temperature;
  uint16_t voltage;
  /** raw encoder counts; i.e. overflown, not consecutive */
  int raw_left_ticks, raw_right_ticks;
  /** absolute encoder counts; i.e. not overflown, but consecutive */
  int diff_left_ticks, diff_right_ticks;
};
extern SensorData g_sensor_data;

/**
 * Callback that fills the sensor data when data is available
 */
struct DataAvailable {
  int latest_ticks_left_, latest_ticks_right_;

  void init(int init_ticks_left, int init_ticks_right);
  void cb(int state);
};
extern DataAvailable g_data_available;

/**
 * Global functions, implemented in stuff.cpp
 */
QString charge_text();
QString global_log_text();
void log(QString text);
int nearest_diff(unsigned short last, unsigned short current);

#endif // STUFF_H_
