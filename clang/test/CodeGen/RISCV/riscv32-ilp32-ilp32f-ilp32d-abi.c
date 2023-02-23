// RUN: %clang_cc1 -triple riscv32 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple riscv32 -target-feature +f -target-abi ilp32f -emit-llvm %s -o - \
// RUN:     | FileCheck %s
// RUN: %clang_cc1 -triple riscv32 -target-feature +d -target-feature +f -target-abi ilp32d -emit-llvm %s -o - \
// RUN:     | FileCheck %s

// This file contains test cases that will have the same output for the ilp32,
// ilp32f, and ilp32d ABIs.

#include <stddef.h>
#include <stdint.h>

struct tiny {
  uint8_t a, b, c, d;
};

struct small {
  int32_t a, *b;
};

struct small_aligned {
  int64_t a;
};

struct large {
  int32_t a, b, c, d;
};

// Ensure that scalars passed on the stack are still determined correctly in
// the presence of large return values that consume a register due to the need
// to pass a pointer.

// CHECK-LABEL: define{{.*}} void @f_scalar_stack_2(ptr noalias sret(%struct.large) align 4 %agg.result, i32 noundef %a, i64 noundef %b, i64 noundef %c, fp128 noundef %d, i8 noundef zeroext %e, i8 noundef signext %f, i8 noundef zeroext %g)
struct large f_scalar_stack_2(int32_t a, int64_t b, int64_t c, long double d,
                              uint8_t e, int8_t f, uint8_t g) {
  return (struct large){a, e, f, g};
}

// CHECK-LABEL: define{{.*}} fp128 @f_scalar_stack_4(i32 noundef %a, i64 noundef %b, i64 noundef %c, fp128 noundef %d, i8 noundef zeroext %e, i8 noundef signext %f, i8 noundef zeroext %g)
long double f_scalar_stack_4(int32_t a, int64_t b, int64_t c, long double d,
                             uint8_t e, int8_t f, uint8_t g) {
  return d;
}

// An "aligned" register pair (where the first register is even-numbered) is
// used to pass varargs with 2x xlen alignment and 2x xlen size. Ensure the
// correct offsets are used.

// CHECK-LABEL: @f_va_2(
// CHECK:         [[FMT_ADDR:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[VA:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[V:%.*]] = alloca double, align 8
// CHECK-NEXT:    store ptr [[FMT:%.*]], ptr [[FMT_ADDR]], align 4
// CHECK-NEXT:    call void @llvm.va_start(ptr [[VA]])
// CHECK-NEXT:    [[ARGP_CUR:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint ptr [[ARGP_CUR]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = add i32 [[TMP0]], 7
// CHECK-NEXT:    [[TMP2:%.*]] = and i32 [[TMP1]], -8
// CHECK-NEXT:    [[ARGP_CUR_ALIGNED:%.*]] = inttoptr i32 [[TMP2]] to ptr
// CHECK-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR_ALIGNED]], i32 8
// CHECK-NEXT:    store ptr [[ARGP_NEXT]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP4:%.*]] = load double, ptr [[ARGP_CUR_ALIGNED]], align 8
// CHECK-NEXT:    store double [[TMP4]], ptr [[V]], align 8
// CHECK-NEXT:    call void @llvm.va_end(ptr [[VA]])
// CHECK-NEXT:    [[TMP5:%.*]] = load double, ptr [[V]], align 8
// CHECK-NEXT:    ret double [[TMP5]]
double f_va_2(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  double v = __builtin_va_arg(va, double);
  __builtin_va_end(va);

  return v;
}

// Two "aligned" register pairs.

// CHECK-LABEL: @f_va_3(
// CHECK:         [[FMT_ADDR:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[VA:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[V:%.*]] = alloca double, align 8
// CHECK-NEXT:    [[W:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[X:%.*]] = alloca double, align 8
// CHECK-NEXT:    store ptr [[FMT:%.*]], ptr [[FMT_ADDR]], align 4
// CHECK-NEXT:    call void @llvm.va_start(ptr [[VA]])
// CHECK-NEXT:    [[ARGP_CUR:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint ptr [[ARGP_CUR]] to i32
// CHECK-NEXT:    [[TMP1:%.*]] = add i32 [[TMP0]], 7
// CHECK-NEXT:    [[TMP2:%.*]] = and i32 [[TMP1]], -8
// CHECK-NEXT:    [[ARGP_CUR_ALIGNED:%.*]] = inttoptr i32 [[TMP2]] to ptr
// CHECK-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR_ALIGNED]], i32 8
// CHECK-NEXT:    store ptr [[ARGP_NEXT]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP4:%.*]] = load double, ptr [[ARGP_CUR_ALIGNED]], align 8
// CHECK-NEXT:    store double [[TMP4]], ptr [[V]], align 8
// CHECK-NEXT:    [[ARGP_CUR2:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT3:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR2]], i32 4
// CHECK-NEXT:    store ptr [[ARGP_NEXT3]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr [[ARGP_CUR2]], align 4
// CHECK-NEXT:    store i32 [[TMP6]], ptr [[W]], align 4
// CHECK-NEXT:    [[ARGP_CUR4:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP7:%.*]] = ptrtoint ptr [[ARGP_CUR4]] to i32
// CHECK-NEXT:    [[TMP8:%.*]] = add i32 [[TMP7]], 7
// CHECK-NEXT:    [[TMP9:%.*]] = and i32 [[TMP8]], -8
// CHECK-NEXT:    [[ARGP_CUR4_ALIGNED:%.*]] = inttoptr i32 [[TMP9]] to ptr
// CHECK-NEXT:    [[ARGP_NEXT5:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR4_ALIGNED]], i32 8
// CHECK-NEXT:    store ptr [[ARGP_NEXT5]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP11:%.*]] = load double, ptr [[ARGP_CUR4_ALIGNED]], align 8
// CHECK-NEXT:    store double [[TMP11]], ptr [[X]], align 8
// CHECK-NEXT:    call void @llvm.va_end(ptr [[VA]])
// CHECK-NEXT:    [[TMP12:%.*]] = load double, ptr [[V]], align 8
// CHECK-NEXT:    [[TMP13:%.*]] = load double, ptr [[X]], align 8
// CHECK-NEXT:    [[ADD:%.*]] = fadd double [[TMP12]], [[TMP13]]
// CHECK-NEXT:    ret double [[ADD]]
double f_va_3(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  double v = __builtin_va_arg(va, double);
  int w = __builtin_va_arg(va, int);
  double x = __builtin_va_arg(va, double);
  __builtin_va_end(va);

  return v + x;
}

// CHECK-LABEL: define{{.*}} i32 @f_va_4(ptr noundef %fmt, ...) {{.*}} {
// CHECK:         [[FMT_ADDR:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[VA:%.*]] = alloca ptr, align 4
// CHECK-NEXT:    [[V:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[LD:%.*]] = alloca fp128, align 16
// CHECK-NEXT:    [[TS:%.*]] = alloca [[STRUCT_TINY:%.*]], align 1
// CHECK-NEXT:    [[SS:%.*]] = alloca [[STRUCT_SMALL:%.*]], align 4
// CHECK-NEXT:    [[LS:%.*]] = alloca [[STRUCT_LARGE:%.*]], align 4
// CHECK-NEXT:    [[RET:%.*]] = alloca i32, align 4
// CHECK-NEXT:    store ptr [[FMT:%.*]], ptr [[FMT_ADDR]], align 4
// CHECK-NEXT:    call void @llvm.va_start(ptr [[VA]])
// CHECK-NEXT:    [[ARGP_CUR:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR]], i32 4
// CHECK-NEXT:    store ptr [[ARGP_NEXT]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr [[ARGP_CUR]], align 4
// CHECK-NEXT:    store i32 [[TMP1]], ptr [[V]], align 4
// CHECK-NEXT:    [[ARGP_CUR2:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT3:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR2]], i32 4
// CHECK-NEXT:    store ptr [[ARGP_NEXT3]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP3:%.*]] = load ptr, ptr [[ARGP_CUR2]], align 4
// CHECK-NEXT:    [[TMP4:%.*]] = load fp128, ptr [[TMP3]], align 16
// CHECK-NEXT:    store fp128 [[TMP4]], ptr [[LD]], align 16
// CHECK-NEXT:    [[ARGP_CUR4:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT5:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR4]], i32 4
// CHECK-NEXT:    store ptr [[ARGP_NEXT5]], ptr [[VA]], align 4
// CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i32(ptr align 1 [[TS]], ptr align 4 [[ARGP_CUR4]], i32 4, i1 false)
// CHECK-NEXT:    [[ARGP_CUR6:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT7:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR6]], i32 8
// CHECK-NEXT:    store ptr [[ARGP_NEXT7]], ptr [[VA]], align 4
// CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i32(ptr align 4 [[SS]], ptr align 4 [[ARGP_CUR6]], i32 8, i1 false)
// CHECK-NEXT:    [[ARGP_CUR8:%.*]] = load ptr, ptr [[VA]], align 4
// CHECK-NEXT:    [[ARGP_NEXT9:%.*]] = getelementptr inbounds i8, ptr [[ARGP_CUR8]], i32 4
// CHECK-NEXT:    store ptr [[ARGP_NEXT9]], ptr [[VA]], align 4
// CHECK-NEXT:    [[TMP12:%.*]] = load ptr, ptr [[ARGP_CUR8]], align 4
// CHECK-NEXT:    call void @llvm.memcpy.p0.p0.i32(ptr align 4 [[LS]], ptr align 4 [[TMP12]], i32 16, i1 false)
// CHECK-NEXT:    call void @llvm.va_end(ptr [[VA]])
int f_va_4(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  int v = __builtin_va_arg(va, int);
  long double ld = __builtin_va_arg(va, long double);
  struct tiny ts = __builtin_va_arg(va, struct tiny);
  struct small ss = __builtin_va_arg(va, struct small);
  struct large ls = __builtin_va_arg(va, struct large);
  __builtin_va_end(va);

  int ret = (int)((long double)v + ld);
  ret = ret + ts.a + ts.b + ts.c + ts.d;
  ret = ret + ss.a + (int)ss.b;
  ret = ret + ls.a + ls.b + ls.c + ls.d;

  return ret;
}
