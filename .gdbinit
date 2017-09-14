set tcp connect-timeout 30
target remote localhost:9000
break kernel_main
continue
