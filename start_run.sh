#! /system/bin/sh

export PATH=/data/busybox/:$PATH

umount /sys/kernel/debug
mount -r -w -t debugfs nodev /sys/kernel/debug

echo sched_switch > /sys/kernel/debug/tracing/current_tracer
#echo nop > /sys/kernel/debug/tracing/current_tracer

cd /data

#chmod 777 iozone
chmod 777 jackfish

#./iozone -Ra &

./jackfish -f /sys/kernel/debug/tracing/trace_pipe

#echo nop > /sys/kernel/debug/tracing/current_tracer
