
#include <iostream>
#include <iomanip>

#include "external_interface/pc/pc_os_model.h"
#include "external_interface/pc/pc_timer.h"

using namespace wiselib;

typedef PCOsModel Os;

Os::Timer *timer;

class MyClass {
	public:
		MyClass() {
			timer = new Os::Timer();
			timer->set_timer<MyClass, &MyClass::on_time>(3000, this, (void*)timer);
		}
		
		~MyClass() {
			delete timer;
		}
		
		void on_time(void*) {
			std::cout << "It's time!" << std::endl;
			timer->set_timer<MyClass, &MyClass::on_time>(1000, this, (void*)timer);
		}
	
	private:
		Os::Timer *timer;
};


int main(int argc, char** argv) {
	
	MyClass c;
	
	// Loop forever in a resource-efficient way
	// so timer events will actually occur
	while(true) pause();
	
	return 0;
}


