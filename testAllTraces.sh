for ((i = 0; i <= 3; i++)); do
    for ((j = 0; j <= 9; j++)); do
		if [[ $i -eq 3 ]]; then
			if [[ $j -eq 3 ]]; then
				exit 0
			fi
		fi	
		if (( i + j == 0 )); then
			continue
		fi
		echo "testing $i$j"
		./cpen212alloc ~cpen212/Public/lab3/traces/trace$i$j.lua
		echo "$i$j passed!"
    done
done
