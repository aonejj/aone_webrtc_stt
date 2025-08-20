#define LOG_NDEBUG 0
#define LOG_TAG "SYSTEM_CORE_TEST"
#include <utils/Log.h>
#include <utils/RefBase.h>

#include <cmath>
#include <limits>
#include <memory>	// for xxx_ptr

#include <fcntl.h>
#include <sys/ioctl.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <sys/eventfd.h>

//using namespace std;
using namespace android;

struct TestClass : public RefBase {
public:
	TestClass() { 
		ALOGV("TestClass::TestClass"); 
	}
	virtual ~TestClass() { 
		ALOGV("TestClass::~TestClass"); 
	}

public:
	virtual void vTestFn() { 
		ALOGV("TestClass::vTestFn"); 
	}
	void TestFn() { 
		ALOGV("TestClass::TestFn"); 
	}
};

void check_sp() {
	ALOGV("check_sp+++");
	sp<TestClass> tmp = new TestClass();
	tmp->vTestFn();
	tmp->TestFn();
	ALOGV("check_sp---");
}

int GreatestCommonDivisor(int a, int b) {
	int c = a % b;
	while (c != 0) {
		a = b;
		b = c;
		c = a % b;
	}
	return b;
}

struct Fraction {
	int numerator;
	int denominator;

	void DivideByGcd() {
		int g = GreatestCommonDivisor(numerator, denominator);
		numerator /= g;
		denominator /= g;
	}

	// Determines number of output pixels if both width and height of an input of
	// |input_pixels| pixels is scaled with the fraction numerator / denominator.
	int scale_pixel_count(int input_pixels) {
		return (numerator * numerator * input_pixels) / (denominator * denominator);
	}
};

void fcntl_test() {
	int mode;
	int r;
	int fd, userfd;
	fd = epoll_create(256);
	ALOGV("fcntl_test epoll_create fd %d", fd);
	
	mode = fcntl(fd, F_GETFL);

	ALOGV("fcntl_test mode %d");

	int set = 1;
	r = ioctl(fd, FIONBIO, &set);

	ALOGV("fcntl_test ioctl ret %d", r);

	userfd = open("./for_test.dump", O_CREAT);
	ALOGV("userfd %d", userfd);

	int pipefds[2] = { -1, -1 };
	pipe(pipefds);

	ALOGV("pipefd [0] %d [1] %d", pipefds[0], pipefds[1]);

	int efd = eventfd(0, 0);

	ALOGV("eventfd %d", efd);
}

class StdSmartPointerTester {
public:
	StdSmartPointerTester() { ALOGV("StdSmartPointerTester"); }
	~StdSmartPointerTester() { ALOGV("~StdSmartPointerTester"); }
};

void std_smart_pointer_test_fn() {
	ALOGV("std_smart_pointer_test_fn+++");
	std::unique_ptr<StdSmartPointerTester> s(new StdSmartPointerTester());
	ALOGV("std_smart_pointer_test_fn---");
}

int main() {
	ALOGV("System core test main+++");
	check_sp();
	ALOGV("System core test main---");

	Fraction fraction;
	fraction.numerator = 1096; // 1280;
	fraction.denominator = 616;// 720;
	fraction.DivideByGcd();

	ALOGV("fraction numerator %d denominator %d", fraction.numerator, fraction.denominator);

	fcntl_test();

	std_smart_pointer_test_fn();

	return 0;
}