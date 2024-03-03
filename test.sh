#!/bin/bash

#declare alll the variables
test_count=0
redis_benchmark_command=""
test_folder=""

main(){
    redis_conf=""
    results_folder=""
    redis_conf="$test_folder/redis.conf"
    redis_pid=""
    redis_benchmark_pid=""



    declare -a monitor_pid

    mkdir "$test_folder/$test_folder-$test_count"

    results_folder="$test_folder/$test_folder-$test_count"

    rm "$results_folder/memoryMonitor.csv"
    rm "$results_folder/CPUMonitor.csv"
    rm "$results_folder/output.txt"
    rm "$results_folder/redis.log"

    > "$results_folder/memoryMonitor.csv"
    > "$results_folder/CPUMonitor.csv"
    > "$results_folder/output.txt"
    > "$results_folder/redis.log"

    ./redis-server "$redis_conf" > "$results_folder/redis.log" 2>&1 &
    redis_pid=($!)

    sleep 2

    ./redis-cli flushall

    ./memoryMonitor "$results_folder/memoryMonitor.csv" &
    monitor_pid+=($!)
    #./memoryMonitorPID "$redis_pid" "$results_folder/memoryMonitorPID.csv" &
    #monitor_pid+=($!)
    ./CPUMonitor "$results_folder/CPUMonitor.csv" &
    monitor_pid+=($!)
    #./CPUMonitorPID "$redis_pid" "$results_folder/CPUMonitorPID.csv" &
    #monitor_pid+=($!)

    temp_file=$(mktemp)
    last_position=0
    current_size=0
    new_content_size=0
    $redis_benchmark_command > "$temp_file" &
    redis_benchmark_pid=($!)

    while ps -p $redis_benchmark_pid > /dev/null; do
        sleep 1
        current_size=$(stat -c %s "$temp_file")
        new_content_size=$((current_size - last_position))
        if [ $new_content_size -gt 0 ]; then
            timestamp=$(date +"%Y-%m-%d %H:%M:%S")
            echo "$timestamp" >> "$results_folder/output.txt"
            tail -c +$last_position "$temp_file" | tr -d '\000' >> "$results_folder/output.txt"

            last_position=$current_size
        fi
    done
    timestamp=$(date +"%Y-%m-%d %H:%M:%S")
    echo "$timestamp" >> "$results_folder/output.txt"
    tail -c +$last_position "$temp_file" | tr -d '\000' >> "$results_folder/output.txt"

    rm "$temp_file"

    sleep 2

    ./redis-cli flushall

    ./redis-cli shutdown

    for pid in "${monitor_pid[@]}"; do
        kill "$pid"
    done

    rm dump.rdb
}

redis_benchmark_command="./redis-benchmark -t set -r 9000 -n 2500000 -d 2097152"
test_folder="test8"
test_count=0
for ((i=0; i<5; i++)); do
    main
    test_count=$((test_count+1))
done

redis_benchmark_command="./redis-benchmark -t set -r 4500 -n 1250000 -d 4194304"
test_folder="test9"
test_count=0
for ((i=0; i<5; i++)); do
    main
    test_count=$((test_count+1))
done

redis_benchmark_command="./redis-benchmark -t set -r 2250 -n 300000 -d 8388608"
test_folder="test10"
test_count=0
for ((i=0; i<5; i++)); do
    main
    test_count=$((test_count+1))
done

#redis_benchmark_command="./redis-benchmark -t set -r 1125 -n 312500 -d 16777216"
#test_folder="test11"
#test_count=0
#for ((i=0; i<5; i++)); do
#    main
#    test_count=$((test_count+1))
#done