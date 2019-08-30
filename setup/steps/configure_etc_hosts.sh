#!/bin/bash
read -d '' ETC_HOSTS <<'HERE'
127.0.0.1      localhost localhost.localdomain localhost4 localhost4.localdomain4
::1            localhost localhost.localdomain localhost6 localhost6.localdomain6
192.168.0.10   exao1 aoc
192.168.0.11   exao2 rtc
192.168.0.12   exao3 icc
192.168.0.140  pdu0
192.168.0.141  pdu1
192.168.0.142  pdu2
192.168.0.150   acromag dio
192.168.0.242   dmsafety
HERE
echo $ETC_HOSTS | sudo tee /etc/hosts
