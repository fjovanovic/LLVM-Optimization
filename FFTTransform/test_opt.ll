; ModuleID = 'test_opt.bc'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [19 x i8] c"First 5 elements: \00", align 1
@.str.1 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"\09Time: %f s\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @multiply_arrays1(ptr noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3, ptr noundef %4) #0 {
entry:
  call void @fft_polynomial_mul_void(ptr %0, i32 %1, ptr %2, i32 %3, ptr %4)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local ptr @multiply_arrays2(ptr noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3) #0 {
entry:
  %4 = call ptr @fft_polynomial_mul(ptr %0, i32 %1, ptr %2, i32 %3)
  ret ptr %4
}

; Function Attrs: nounwind allocsize(0)
declare noalias ptr @malloc(i64 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i64, align 8
  %10 = alloca i64, align 8
  %11 = alloca double, align 8
  %12 = alloca i32, align 4
  %13 = alloca i64, align 8
  %14 = alloca ptr, align 8
  %15 = alloca i64, align 8
  %16 = alloca double, align 8
  %17 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i32 72000, ptr %2, align 4
  store i32 34000, ptr %3, align 4
  %18 = load i32, ptr %2, align 4
  %19 = sext i32 %18 to i64
  %20 = mul i64 %19, 4
  %21 = call noalias ptr @malloc(i64 noundef %20) #4
  store ptr %21, ptr %4, align 8
  %22 = load i32, ptr %3, align 4
  %23 = sext i32 %22 to i64
  %24 = mul i64 %23, 4
  %25 = call noalias ptr @malloc(i64 noundef %24) #4
  store ptr %25, ptr %5, align 8
  %26 = load i32, ptr %2, align 4
  %27 = load i32, ptr %3, align 4
  %28 = add nsw i32 %26, %27
  %29 = sub nsw i32 %28, 1
  %30 = sext i32 %29 to i64
  %31 = mul i64 %30, 4
  %32 = call noalias ptr @malloc(i64 noundef %31) #4
  store ptr %32, ptr %6, align 8
  call void @srand(i32 noundef 42) #5
  store i32 0, ptr %7, align 4
  br label %33

33:                                               ; preds = %45, %0
  %34 = load i32, ptr %7, align 4
  %35 = load i32, ptr %2, align 4
  %36 = icmp slt i32 %34, %35
  br i1 %36, label %37, label %48

37:                                               ; preds = %33
  %38 = call i32 @rand() #5
  %39 = srem i32 %38, 100
  %40 = add nsw i32 %39, 1
  %41 = load ptr, ptr %4, align 8
  %42 = load i32, ptr %7, align 4
  %43 = sext i32 %42 to i64
  %44 = getelementptr inbounds i32, ptr %41, i64 %43
  store i32 %40, ptr %44, align 4
  br label %45

45:                                               ; preds = %37
  %46 = load i32, ptr %7, align 4
  %47 = add nsw i32 %46, 1
  store i32 %47, ptr %7, align 4
  br label %33, !llvm.loop !6

48:                                               ; preds = %33
  store i32 0, ptr %8, align 4
  br label %49

49:                                               ; preds = %61, %48
  %50 = load i32, ptr %8, align 4
  %51 = load i32, ptr %3, align 4
  %52 = icmp slt i32 %50, %51
  br i1 %52, label %53, label %64

53:                                               ; preds = %49
  %54 = call i32 @rand() #5
  %55 = srem i32 %54, 100
  %56 = add nsw i32 %55, 1
  %57 = load ptr, ptr %5, align 8
  %58 = load i32, ptr %8, align 4
  %59 = sext i32 %58 to i64
  %60 = getelementptr inbounds i32, ptr %57, i64 %59
  store i32 %56, ptr %60, align 4
  br label %61

61:                                               ; preds = %53
  %62 = load i32, ptr %8, align 4
  %63 = add nsw i32 %62, 1
  store i32 %63, ptr %8, align 4
  br label %49, !llvm.loop !8

64:                                               ; preds = %49
  %65 = call i64 @clock() #5
  store i64 %65, ptr %9, align 8
  %66 = load ptr, ptr %4, align 8
  %67 = load i32, ptr %2, align 4
  %68 = load ptr, ptr %5, align 8
  %69 = load i32, ptr %3, align 4
  %70 = load ptr, ptr %6, align 8
  call void @multiply_arrays1(ptr noundef %66, i32 noundef %67, ptr noundef %68, i32 noundef %69, ptr noundef %70)
  %71 = call i64 @clock() #5
  store i64 %71, ptr %10, align 8
  %72 = load i64, ptr %10, align 8
  %73 = load i64, ptr %9, align 8
  %74 = sub nsw i64 %72, %73
  %75 = sitofp i64 %74 to double
  %76 = fdiv double %75, 1.000000e+06
  store double %76, ptr %11, align 8
  %77 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 0, ptr %12, align 4
  br label %78

78:                                               ; preds = %88, %64
  %79 = load i32, ptr %12, align 4
  %80 = icmp slt i32 %79, 5
  br i1 %80, label %81, label %91

81:                                               ; preds = %78
  %82 = load ptr, ptr %6, align 8
  %83 = load i32, ptr %12, align 4
  %84 = sext i32 %83 to i64
  %85 = getelementptr inbounds i32, ptr %82, i64 %84
  %86 = load i32, ptr %85, align 4
  %87 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %86)
  br label %88

88:                                               ; preds = %81
  %89 = load i32, ptr %12, align 4
  %90 = add nsw i32 %89, 1
  store i32 %90, ptr %12, align 4
  br label %78, !llvm.loop !9

91:                                               ; preds = %78
  %92 = load double, ptr %11, align 8
  %93 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, double noundef %92)
  %94 = call i64 @clock() #5
  store i64 %94, ptr %13, align 8
  %95 = load ptr, ptr %4, align 8
  %96 = load i32, ptr %2, align 4
  %97 = load ptr, ptr %5, align 8
  %98 = load i32, ptr %3, align 4
  %99 = call ptr @multiply_arrays2(ptr noundef %95, i32 noundef %96, ptr noundef %97, i32 noundef %98)
  store ptr %99, ptr %14, align 8
  %100 = call i64 @clock() #5
  store i64 %100, ptr %15, align 8
  %101 = load i64, ptr %15, align 8
  %102 = load i64, ptr %13, align 8
  %103 = sub nsw i64 %101, %102
  %104 = sitofp i64 %103 to double
  %105 = fdiv double %104, 1.000000e+06
  store double %105, ptr %16, align 8
  %106 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 0, ptr %17, align 4
  br label %107

107:                                              ; preds = %117, %91
  %108 = load i32, ptr %17, align 4
  %109 = icmp slt i32 %108, 5
  br i1 %109, label %110, label %120

110:                                              ; preds = %107
  %111 = load ptr, ptr %14, align 8
  %112 = load i32, ptr %17, align 4
  %113 = sext i32 %112 to i64
  %114 = getelementptr inbounds i32, ptr %111, i64 %113
  %115 = load i32, ptr %114, align 4
  %116 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %115)
  br label %117

117:                                              ; preds = %110
  %118 = load i32, ptr %17, align 4
  %119 = add nsw i32 %118, 1
  store i32 %119, ptr %17, align 4
  br label %107, !llvm.loop !10

120:                                              ; preds = %107
  %121 = load double, ptr %16, align 8
  %122 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, double noundef %121)
  %123 = load ptr, ptr %14, align 8
  call void @free(ptr noundef %123) #5
  %124 = load ptr, ptr %4, align 8
  call void @free(ptr noundef %124) #5
  %125 = load ptr, ptr %5, align 8
  call void @free(ptr noundef %125) #5
  %126 = load ptr, ptr %6, align 8
  call void @free(ptr noundef %126) #5
  ret i32 0
}

; Function Attrs: nounwind
declare void @srand(i32 noundef) #2

; Function Attrs: nounwind
declare i32 @rand() #2

; Function Attrs: nounwind
declare i64 @clock() #2

declare i32 @printf(ptr noundef, ...) #3

; Function Attrs: nounwind
declare void @free(ptr noundef) #2

declare void @fft_polynomial_mul_void(ptr, i32, ptr, i32, ptr)

declare ptr @fft_polynomial_mul(ptr, i32, ptr, i32)

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind allocsize(0) }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.0"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
