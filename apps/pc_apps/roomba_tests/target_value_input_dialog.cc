/**
 * @file my_input_dialog.cpp
 * @date 8 Mar 2011
 * @author Roland Hieber <rohieb@rohieb.name>
 */

#include "stuff.h"
#include "target_value_input_dialog.h"
#include <limits>

using namespace std;

QString const TargetValueInputDialog::orientation_text_ =
  "Target orientation: %1 degree\nCurrent real orientation: %2 (%3%4)";

TargetValueInputDialog::TargetValueInputDialog() :
  p_id_(0), last_orientation_(0), target_orientation_(0) {
  p_id_ = new QInputDialog;
  p_id_->setInputMode(QInputDialog::IntInput);
  p_id_->setIntRange(numeric_limits<int>::min(), numeric_limits<int>::max());
  p_id_->setIntStep(1);
}

TargetValueInputDialog::~TargetValueInputDialog() {
}

/**
 * Display a dialog to allow the user the input of the current orientation
 * @param text additional text to be displayed before the input box
 * @param last the orientation before the turn iteration
 * param target the target orientation after the turn iteration
 * @param ok If this parameter is non-null, it will be set to @c true if the
 *  user pressed OK and to @c false if the user pressed Cancel.
 * @return On success, this function returns the integer which has been entered
 *  by the user; on failure, it returns the initial angle.
 */
int TargetValueInputDialog::orientation(const QString text, const int last,
  const int target, bool * ok) {
  last_orientation_ = last;
  target_orientation_ = target;
  text_ = text;

  connect(p_id_, SIGNAL(intValueChanged(int)), this,
    SLOT(turn_dialog_value_changed(int)));

  p_id_->setWindowTitle("Input orientation");
  p_id_->setLabelText(
    orientation_text_.arg(target).arg((target + 360) % 360).arg("+0") + "\n\n"
      + text);
  p_id_->setIntValue(target - last);
  int ret = p_id_->exec();
  if(ok) {
    *ok = (bool)ret;
  }

  disconnect(p_id_, SIGNAL(intValueChanged(int)), this,
    SLOT(turn_dialog_value_changed(int)));

  if(ret) {
    return p_id_->intValue();
  } else {
    return target - last;
  }
}

/**
 * Callback that updates the text in the dialog box when the value is changed
 */
void TargetValueInputDialog::turn_dialog_value_changed(int new_value) {
  int diff = new_value - (target_orientation_ - last_orientation_);

  p_id_->setLabelText(orientation_text_.arg(target_orientation_).arg(
    (last_orientation_ + new_value + 360) % 360).arg(diff < 0 ? "" : "+").arg(
    diff) + "\n\n" + text_);
}
