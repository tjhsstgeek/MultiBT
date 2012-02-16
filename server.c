#include "bubbletrouble.h"

int main() {
	if (network_start()) {
		ERR_TRACE();
		return -1;
	}
	if (network_create()) {
		ERR_TRACE();
		return -1;
	}
	return 0;
}
