#include <stdio.h>
#include <stdlib.h>
#include "../Include/arrayobject.h"

void array_test1() {
	ArrayObject *a = Array_New(0, sizeof(int));
	assert(SIZE(a) == 0);
	int i;
	const int n = 100000;
	for (i = 0; i < n; ++i) {
		Array_Append(a, &i);
	}
	assert(SIZE(a) == n);
	assert(Array_Find(a, (int[]) {6776}, 0) == 6776);
	assert(Array_Find(a, (int[]) {8}, 30001) == SIZE(a));
	for (i = 0; i < 1000; ++i) {
		Array_Remove(a, SIZE(a) - 1);
	}
	assert(SIZE(a) == n - 1000);
	ArrayObject *b = Array_Copy(a);
	Array_Clear(a);
	assert(SIZE(a) == 0);
	assert(SIZE(b) == n - 1000);
	for (i = 0; i < n; ++i) {
		assert(Array_Find(a, (int[]) {i}, 0) == SIZE(a));
	}
	Array_Swap(a, b);
	assert(SIZE(a) == n - 1000);
	assert(SIZE(b) == 0);

	for (i = 0; i < n - 1000; ++i) {
		assert(Array_At(int, a, i) == i);
	}
	DECREF(b);
	DECREF(a);

}

int main() {
	array_test1();
	return 0;
}