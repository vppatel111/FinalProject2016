#ifndef STACK_H
#define STACK_H

#include "ArduinoExtras.h"

struct Stack {
  /** Index of the top item, or -1 if stack is empty. */
  int topIdx;
  /** Dynamically allocated array */
  int *arr;
  char *init_1;
  char *init_2;
  char *init_3;
  /** Size of arr */
  int arrSize;

  Stack() {
    topIdx = -1;
    arr = new int[11];
    init_1 = new char[11];
    init_2 = new char[11];
    init_3 = new char[11];
    arrSize = 11;
  }

  ~Stack() {
    delete[] arr;
  }

  bool isEmpty() {
    return topIdx < 0;
  }

  bool isFull() {
    return topIdx == 10;
  }

  int numItems() {
    return topIdx;
  }

  int top() {
    assert(!isEmpty());
    return arr[topIdx];
  }

  char initial1() {
    assert(!isEmpty());
    return init_1[topIdx];
  }

  char initial2() {
    assert(!isEmpty());
    return init_2[topIdx];
  }

  char initial3() {
    assert(!isEmpty());
    return init_3[topIdx];
  }

  void pop() {
    assert(!isEmpty());
    topIdx--;
  }

  void push(int item, char init1, char init2, char init3) {
    topIdx++;
    arr[topIdx] = item;
    init_1[topIdx] = init1;
    init_2[topIdx] = init2;
    init_3[topIdx] = init3;
  }

};

#endif
