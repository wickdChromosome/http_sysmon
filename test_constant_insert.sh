token='fa37JncCHryDsbzayy4cBWDxS22JjzhM'

for i in $(seq 36000 72000); do 


	host_name='bencepc'
	unix_time=$(date +%s)
	load_avg=$(cut -d ' ' -f2 /proc/loadavg)
	ram_perc=$(free | grep Mem | awk '{print $3/$2 * 100.0}')

	echo ${i}
	echo ${ram_perc}
	curl -d "access_token=${token}&load_avg=${load_avg}&ram_perc=${ram_perc}&host_name=${host_name}&unix_time=${unix_time}" -X POST 127.0.0.1:8080/report
	sleep 5;

done

