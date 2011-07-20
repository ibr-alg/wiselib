/**
 * @file my_input_dialog.h
 * @date 8 Mar 2011
 * @author Roland Hieber <rohieb@rohieb.name>
 */

#ifndef MY_INPUT_DIALOG_H_
#define MY_INPUT_DIALOG_H_

#include <QInputDialog>
#include <QString>

class TargetValueInputDialog : QObject {
Q_OBJECT

public:
  TargetValueInputDialog();
  virtual ~TargetValueInputDialog();
  int orientation(QString const text, const int cur_orientation, const int angle,
    bool * ok);

public slots:
  void turn_dialog_value_changed(int new_value);

private:
  QInputDialog * p_id_;
  QString text_;
  int last_orientation_;
  int target_orientation_;

  static QString const orientation_text_;
};

#endif /* MY_INPUT_DIALOG_H_ */
