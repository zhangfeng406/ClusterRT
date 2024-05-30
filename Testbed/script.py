import os
import sys
import subprocess

ips = ["172.31.0.202", "172.31.0.203", "172.31.0.204", "172.31.0.205", "172.31.0.206", "172.31.0.207", "172.31.0.208", "172.31.0.209", "172.31.0.210", "172.31.0.211", 
"172.31.0.212", "172.31.0.213"]

otherIPs = ["172.17.205.235", "172.17.205.236", "172.17.205.237", "172.17.205.238", "172.17.205.239", "172.17.205.240", "172.17.205.241", "172.17.205.242", "172.17.205.243"
, "172.17.205.244", "172.17.205.245", "172.17.205.246", "172.17.205.247", "172.17.205.248", "172.17.205.249", "172.17.205.250", "172.17.205.251", "172.17.205.252"]
workIPs = ["192.168.0.202","192.168.0.203","192.168.0.204","192.168.0.205"]

nodeSet = ["Slave2", "Slave3", "Slave4", "Slave5", "Slave6", "Slave7", "Slave8", "Slave9", "Slave10", "Slave11", "Slave12", "Slave13"]
ycsbSet = ["Slave10", "Slave11", "Slave12", "Slave13"]
failNode = "Slave2"

ycsbHome = "/root/ycsb-0.17.0"
hbaseHome = "/root/hbase-2.3.6"


onlyFore = "0"
onlyRepair = "1"
foreAndRepair = "2"

sendFilePath = "~/ClusterRT"
receiveFilePath = "/root/"

files = "*.h *.cpp *.c Makefile"

def mkwenjian():
    for node in otherIPs:
        command = "ssh " + node + " \"" + "mkdir zf;cd zf;mkdir ClusterRT\" "
        print(command)
        os.system(command)

def installg():
    for node in otherIPs:
        command = "ssh " + node + " \"" + "yum install g++\" "
        print(command)
        os.system(command)

def delspeedCtrl():
    for node in otherIPs:
        command = "ssh " + node + " \"" + "cd " + sendFilePath +"; sudo tc qdisc del dev eno1 root\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def controlSpeedRack():
    for node in otherIPs:
        command = "ssh " + node + " \"" + "cd " + sendFilePath +"; sudo tc qdisc del dev eno1 root; sudo sh 250Mb.sh\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def deploy():
    command = "make clean; rm -f data_file"
    print(command)
    os.system(command)
    
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" scp -o StrictHostKeyChecking=no -r {} root@{}:{}'.format(sendFilePath, node, receiveFilePath)
        print(command)
        os.system(command)

def deployFiles():
    for node in otherIPs:
        command = "scp " + files + " " + node + ":" + sendFilePath
        print(command)
        os.system(command)

def executeMake():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; make clean; make\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])
    command = "make clean; make"
    print(command)
    subprocess.Popen(['/bin/bash', '-c', command])

def executeProxyElastic():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; ./ElasticEC proxy ElasticEC\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def closeProxy():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; killall -9 ElasticEC\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def executeProxyBase():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; ./ElasticEC proxy base\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def executeProxyERS():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; ./ElasticEC proxy ers\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def executeProxySRS():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; ./ElasticEC proxy srs\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])


def GenDataFile():
    for node in otherIPs:
        command = 'sshpass -p "Aa123456" ssh -o StrictHostKeyChecking=no ' + node + " \"" + "cd " + sendFilePath +"; dd if=/dev/zero of=data_file bs=1M count=1024\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def runOnlyFore():
    for node in nodeSet:
        command = "ssh " + "root@"+ node + " \"" + "cd " + slave_OutputPath + "; rm *.csv; " + "nethogs -t > " + slave_OutputPath + "/" + node + "_output_" + onlyFore + ".csv\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])
    for node in ycsbSet:
        command = "ssh " + "root@"+ node + " \"" + ycsbHome + "/bin/ycsb run hbase20 -P " + ycsbHome + "/workloads/testworkload -cp " + hbaseHome + "/conf/ -threads 4\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

def closeNethogs():
    for node in nodeSet:
        command = "ssh " + "root@"+ node + " \"killall nethogs\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])
    for node in nodeSet:
        command = "ssh " + "root@"+ node + " \"scp " + slave_OutputPath + "/*.csv Master:~/ggw/output/\" "
        print(command)
        os.system(command)

def runOnlyRepair():
    for node in nodeSet:
        command = "ssh " + "root@"+ node + " \"" + "cd " + slave_OutputPath + "; rm *.csv; " + "nethogs -t > " + slave_OutputPath + "/" + node + "_output_" + onlyRepair + ".csv\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])

    command = "python3 " + repairboostPath + "/scripts/start.py"
    print(command)
    os.system(command)

    command = command = "ssh " + "root@"+ failNode + " \"cd " + repairboostPath + "; ./ECClient\" "
    print(command)
    os.system(command)

    command = "python3 " + repairboostPath + "/scripts/stop.py"
    print(command)
    os.system(command)
    
def runForeAndRepair():
    for node in nodeSet:
        command = "ssh " + "root@"+ node + " \"" + "cd " + slave_OutputPath + "; rm *.csv; " + "nethogs -t > " + slave_OutputPath + "/" + node + "_output_" + foreAndRepair + ".csv\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command]) 
    for node in ycsbSet:
        command = "ssh " + "root@"+ node + " \"" + ycsbHome + "/bin/ycsb run hbase20 -P " + ycsbHome + "/workloads/testworkload -cp " + hbaseHome + "/conf/ -threads 4\" "
        print(command)
        subprocess.Popen(['/bin/bash', '-c', command])
    
    command = "python3 " + repairboostPath + "/scripts/start.py"
    print(command)
    os.system(command)

    command = command = "ssh " + "root@"+ failNode + " \"cd " + repairboostPath + "; ./ECClient\" "
    print(command)
    os.system(command)
    
    command = "python3 " + repairboostPath + "/scripts/stop.py"
    print(command)
    os.system(command)


if __name__ == '__main__':
    if(len(sys.argv)) < 2:
        print("argument not enough:")
        print("\ttype: 0 -> deploy; 1 -> make")
        sys.exit() 
    # if(sys.argv[1].isalnum()):
    #     t = (int)(sys.argv[1])
    # else:
    #     t = sys.argv[1]
    t=sys.argv[1]
    print(t)
    if t == "0":
        deployFiles()
    elif t == "make":
        executeMake()
    elif t == "idea":
        executeProxyElastic()
    elif t == "22":
        executeProxyBase()
    elif t == "ers":
        executeProxyERS()
    elif t == "srs":
        executeProxySRS()
    elif t == "close":
        closeProxy()
    elif t == "gen":
        GenDataFile()
    elif t == "deploy":
        deploy()
    elif t == "clear":
        delspeedCtrl()
    elif t == "tc":
        controlSpeedRack()
    elif t == "g":
        installg()
    elif t == "m":
        mkwenjian()
