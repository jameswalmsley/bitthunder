#include <bitthunder.h>

int main(int argc, char **argv) {

	BT_u32 i = 0;

	while(1) {
		i++;
		BT_ThreadSleep(1000);
	}
	return 0;
}

