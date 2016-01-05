#!/bin/bash



while read -r line 
do  
    if [[ $line =~ "1K" ]]; then 
        a=${line%1K}; 
        b="$a"2_0.5_-0.1_1K; 
        echo $b 
    fi

    if [[ $line =~ "10K" ]]; then 
        a=${line%10K}; 
        b="$a"2_0.5_-0.1_10K; 
        echo $b 
    fi

    if [[ $line =~ "100K" ]]; then 
        a=${line%100K}; 
        b="$a"2_0.5_-0.1_100K; 
        echo $b 
    fi

done < order

