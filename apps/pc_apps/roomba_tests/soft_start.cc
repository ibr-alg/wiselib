#include "stuff.h"
#include "soft_start_motion.h"
#include "target_value_input_dialog.h"
#include <limits>
#include <ctime>
#include <QApplication>
#include <QInputDialog>

typedef wiselib::SoftStartMotion<OsModel, Roomba> SoftStartMotion;
SoftStartMotion g_soft_motion;

using namespace std;

/**
 * Do one drive iteration. Logs values to file. If distance and velocity are
 * not given, they are requested from the user.
 * @return false if the user cancelled, true otherwise
 */
bool drive(int distance = 0, int velocity = 0) {

  bool ok = false;

  if(distance == 0) {
    distance = get_int(0, "Input distance", charge_text()
      + "Input new distance in mm:", distance, numeric_limits<int>::min(),
      numeric_limits<int>::max(), 1, &ok);
    if(!ok) {
      return false;
    }
  }
  if(velocity == 0) {
    velocity = get_int(0, "Input velocity", charge_text()
      + "Input drive velocity in mm/sec:", velocity, -500, 500, 10, &ok);
    if(!ok) {
      return false;
    }
  }

  std::cout << "Driving with velocity=" << velocity << " dist=" << distance
    << std::endl;

  g_soft_motion.move_distance(distance, velocity);
  g_roomba.wait_for_stop();

  // measured deviation
  int measured_x = get_int(0, "Input distance", charge_text()
    + "Input travelled distance in the original viewing direction in mm:",
    distance, numeric_limits<int>::min(), numeric_limits<int>::max(), 1, &ok);
  if(!ok) {
    return false;
  }
  int measured_y = get_int(0, "Input distance", charge_text()
    + "Input travelled distance perpendicular to the original viewing "
    + "direction in mm:", 0, numeric_limits<int>::min(),
    numeric_limits<int>::max(), 1, &ok);
  if(!ok) {
    return false;
  }
  int deviation_orientation = get_int(0, "Input orientation", charge_text()
    + "Input difference of orientation in degree:", 0,
    numeric_limits<int>::min(), numeric_limits<int>::max(), 1, &ok);
  if(!ok) {
    return false;
  }

  log(global_log_text() + " move=straight input_distance=" + QString::number(
    distance) + " velocity=" + QString::number(velocity) + " measured_x="
    + QString::number(measured_x) + " measured_y="
    + QString::number(measured_y) + " deviation_orientation="
    + QString::number(deviation_orientation) + " wiselib_internal_distance="
    + QString::number(g_roomba.distance()) + " roomba_internal_distance=");

  // reset, because we only need the difference between two drive commands
  g_sensor_data.diff_left_ticks = 0;
  g_sensor_data.diff_right_ticks = 0;

  return true;
}

/**
 * Do one turn iteration. Logs values to file. If angle and/or velocity are not
 * given, they are requested from the user.
 * @param cur_orientation the current orientation of the Roomba (0-360°)
 * @return false if the user cancelled, true otherwise
 */
bool turn(int& cur_orientation, int velocity = 0, int angle = 0) {
  bool ok = false;

  if(velocity == 0) {
    velocity = get_int(0, "Input velocity", charge_text()
      + "Input turn velocity in mm/sec:", 100, 0, 500, 10, &ok);
    if(!ok) {
      return false;
    }
  }

  if(angle == 0) {
    angle = get_int(0, "Input turn angle", charge_text() + "Input angle in "
      "degree to turn about:", 90, numeric_limits<int>::min() + 360,
      numeric_limits<int>::max() - 360, 1, &ok);
    if(!ok) {
      return false;
    }
  }

  std::cout << "Turning with velocity=" << velocity << " angle=" << angle
    << std::endl;

  g_soft_motion.turn_about(Math::degrees_to_radians(angle), velocity);
  g_roomba.wait_for_stop();

  // new current angle
  //  int measured_angle = get_int(0, "Input measured angle", charge_text()
  //    + QString("Orientation should be %1 degree now.\n\n").arg((cur_orientation
  //      + angle + 360) % 360) + "Input measured angle in degree the Roomba "
  //    "has turned:", angle, numeric_limits<int>::min(),
  //    numeric_limits<int>::max(), 1, &ok);
  static TargetValueInputDialog dlg;
  int measured_angle = dlg.orientation(charge_text() + "Measured angle the "
    "Roomba has turned:", cur_orientation, cur_orientation + angle, &ok);
  if(!ok) {
    return false;
  }

  log(global_log_text() + " move=turn turn_angle=" + QString::number(angle)
    + " measured_angle=" + QString::number(measured_angle) + " velocity="
    + QString::number(velocity) + " wiselib_internal_angle=" + QString::number(
    g_roomba.angle()));

  // reset, because we only need the difference between two turns
  g_sensor_data.diff_left_ticks = 0;
  g_sensor_data.diff_right_ticks = 0;

  // new orientation, avoid negative values
  cur_orientation = (cur_orientation + measured_angle + 360) % 360;

  return true;
}

/**
 * main function
 */
int main(int argc, char ** argv) {
  if(argc < 4 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
    cerr << "Usage: " << argv[0] << " mode roomba-id ground-type" << endl
      << "Where mode is:" << endl
      << "  -t|--turn          Turn with predefined test program" << endl
      << "  -T|--turn-manual   Turn with manual input" << endl
      << "  -d|--drive         Drive with predefined test program" << endl
      << "  -D|--drive-manual  Drive with manual input" << endl << endl
      << "This program was compiled from " << SVNREVISION << endl << endl;
    return -1;
  }

  g_roomba_id = argv[2];
  g_ground_type = argv[3];

  // init stuff
  QApplication app(argc, argv);

  g_roomba_uart.set_baudrate(19200);
  g_roomba_uart.enable_serial_comm();
  g_roomba.init(g_roomba_uart, g_timer, Roomba::POSITION
    | Roomba::BATTERY_AND_TEMPERATURE);

  cerr << "Got roomba at " << g_roomba_uart.address() << endl;

  g_roomba.reset_distance();
  g_roomba.reset_angle();
  g_soft_motion.init(g_roomba, g_timer);

  // we do not want the probably corrupted data from roomba(), instead we fill
  // our own values when data is available
  g_roomba.register_state_callback<DataAvailable, &DataAvailable::cb> (
    &g_data_available);

  // ...and fill it once
  g_roomba.notify_state_receivers(Roomba::DATA_AVAILABLE);
  g_data_available.init(g_sensor_data.raw_left_ticks,
    g_sensor_data.raw_right_ticks);

  // actual tests
  if(strcmp(argv[1], "--turn") == 0 || strcmp(argv[1], "-t") == 0) {
    // time-based log file name
    sprintf(g_log_file, "roomba-soft-stop-turn-%d.log", int(time(NULL)));

    /*********************** pre-programmed turn tests ***********************/
    int orientation = 0;
    bool ok = false;

    // current orientation
    orientation = get_int(0, "Input current orientation", charge_text()
      + "Input current orientation in degree:", orientation, 0, 359, 1, &ok);
    if(!ok) {
      return -1;
    }

    // test runs:
    int velocities[] = { /*20, 50, 70, */100/*, 150, 200, 300, 400*/ };
    int angles[] = { /*5, 15, 30, 45, 90, 120, */180/*, 360, 530, 720*/ };

    // shuffle elements by randomly exchanging each with one other.
    for(size_t i = 0; i < (sizeof(velocities) / sizeof(int) - 1); i++) {
      int r = i + (rand() % (sizeof(velocities) / sizeof(int) - i));
      int temp = velocities[i];
      velocities[i] = velocities[r];
      velocities[r] = temp;
    }
    for(size_t i = 0; i < (sizeof(angles) / sizeof(int) - 1); i++) {
      int r = i + (rand() % (sizeof(angles) / sizeof(int) - i));
      int temp = angles[i];
      angles[i] = angles[r];
      angles[r] = temp;
    }

    for(size_t v = 0; v < sizeof(velocities) / sizeof(int); v++) {
      for(size_t a = 0; a < sizeof(angles) / sizeof(int); a++) {
        if(!turn(orientation, velocities[v], angles[a])) {
          return -1; // user cancelled
        }
      }
    }
    return 0; // finished!

  } else if(strcmp(argv[1], "--turn-manual") == 0 || strcmp(argv[1], "-T") == 0) {
    // time-based log file name
    sprintf(g_log_file, "roomba-soft-stop-turn-%d.log", int(time(NULL)));

    /************************** manual turn tests **************************/
    int orientation = 0;
    bool ok = false;

    // current orientation
    orientation = get_int(0, "Input current orientation", charge_text()
      + "Input current orientation in degree:", orientation, 0, 359, 1, &ok);
    if(!ok) {
      return -1;
    }

    while(ok = turn(orientation))
      ; // until the user cancels
    return -1;

  } else if(strcmp(argv[1], "--drive") == 0 || strcmp(argv[1], "-d") == 0) {
    // time-based log file name
    sprintf(g_log_file, "roomba-soft-stop-drive-%d.log", int(time(NULL)));

    /******************** pre-programmed drive tests **********************/
    // test runs:
    int velocities[] = { /*20,*/ 50, /*70,*/ 100, /*150,*/ 200, 300, 400 };
    int distances[] = { /*20, 50,*/ 100, 200, /*500,*/ 1000, 2000, 4000 };

    // shuffle elements by randomly exchanging each with one other.
    for(size_t i = 0; i < (sizeof(velocities) / sizeof(int) - 1); i++) {
      int r = i + (rand() % (sizeof(velocities) / sizeof(int) - i));
      int temp = velocities[i];
      velocities[i] = velocities[r];
      velocities[r] = temp;
    }
    for(size_t i = 0; i < (sizeof(distances) / sizeof(int) - 1); i++) {
      int r = i + (rand() % (sizeof(distances) / sizeof(int) - i));
      int temp = distances[i];
      distances[i] = distances[r];
      distances[r] = temp;
    }

    for(size_t v = 0; v < sizeof(velocities) / sizeof(int); v++) {
      for(size_t a = 0; a < sizeof(distances) / sizeof(int); a++) {
        if(!drive(distances[a], velocities[v])) {
          return -1; // user cancelled
        }
      }
    }
    return 0; // finished!

  } else if(strcmp(argv[1], "--drive-manual") == 0 || strcmp(argv[1], "-D")
    == 0) {
    // time-based log file name
    sprintf(g_log_file, "roomba-soft-stop-drive-%d.log", int(time(NULL)));

    /************************** manual drive tests **************************/
    bool ok = false;
    while(ok = drive())
      ; // until the user cancels
    return -1;
  }
}
