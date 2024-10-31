; ModuleID = 'MainModule'
source_filename = "MainModule"

define i32 @main() {
main0:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 2, ptr %x, align 4
  store i32 5, ptr %y, align 4
  %0 = load i32, ptr %x, align 4
  %1 = load i32, ptr %y, align 4
  %2 = icmp slt i32 %0, %1
  %3 = alloca i32, align 4
  store i1 %2, ptr %3, align 1
  %4 = load i32, ptr %3, align 4
  ret i32 0
}
