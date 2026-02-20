import serial
import json
import time
import threading

idval = 2
screenval = 2
bmsval = 2
volval = 2
langval = 1
audioval = 1
modeval = 1
pauseval = 0
pausemode = 0
calval = 0
alertval = 0
syserrval = 0

flg_pause = 0
flg_cal = 0
flg_alert = 0
flg_syserr = 0

def serialInit():
	ser = serial.Serial("/dev/ttyACM0",115200)
	return ser

def inc_var():
	global pauseval
	global pausemode
	global flg_pause

	pausemode = 0

	if pauseval >= 0:
		threading.Timer(1,inc_var).start()
		flg_pause = 1
	else:
		flg_pause = 0

	pauseval -= 1


def writeData(ser):
	global idval
	global screenval
	global bmsval
	global volval
	global langval
	global audioval
	global modeval
	global pauseval
	global pausemode
	global flg_pause
	global flg_cal
	global flg_alert
	global flg_syserr
	global calval
	global alertval
	global syserrval

	while True:
		time.sleep(0.2) #wait for 0.2 second

		if screenval == 5:
			screenval = 1

		if pausemode == 1 or pausemode == 2:
			if pausemode == 1:
				pauseval = 301 #5 minutes for short pause
			if pausemode == 2:
				pauseval = 1801 #259200 #3days #1801 #30 minutes for long pause
			inc_var()

		read_Data = ser.readline().decode('utf-8').strip()
		if read_Data != "":
			try:
				json_data = json.loads(read_Data)
				print("received: ", json_data)

				if "method" in json_data :
					if json_data["method"] == "activate_sensor":
						flg_cal = 1
						calval = 1
						screenval = 2
					if json_data["method"] == "deactivate_sensor":
						screenval = 5
					if json_data["method"] == "activate_chair_mode":
						flg_cal = 1
						calval = 2
					#	modeval = 2
						pauseval = -1
					if json_data["method"] == "activate_bed_mode":
						modeval = 1
						pauseval = -1

				if("params" in json_data):
					if("screen" in json_data["params"]):
						screenval = json_data["params"]["screen"]
					if("bms" in json_data["params"]):
						bmsval = json_data["params"]["bms"]
					if("vol" in json_data["params"]):
						volval = json_data["params"]["vol"]
					if("lang" in json_data["params"]):
						langval = json_data["params"]["lang"]
					if("audio" in json_data["params"]):
						audioval = json_data["params"]["audio"]
					if("pause" in json_data["params"]):
						if flg_pause == 0:
							pausemode = json_data["params"]["pause"]

			except json.decoder.JSONDecodeError as e:
				# print("Invalid json data",e)
				print("Debug:", read_Data)  # Print non-JSON debug logs
		try:
			if flg_pause == 1:
				data = {"jsonrpc":"2.0","method":"set_state", "params":{"screen":screenval,"bms":bmsval,"vol":volval, "lang":langval,"audio":audioval,"mode":modeval,"pause_tmr":pauseval,"room_number":"001"}, "id":idval}
			elif flg_cal == 1:
				data = {"jsonrpc":"2.0","method":"set_state", "params":{"screen":screenval,"bms":bmsval,"vol":volval, "lang":langval,"audio":audioval,"mode":modeval,"cal":calval,"room_number":"001"}, "id":idval}
				flg_cal = 0
				# if calval == 1:
				# 	screenval = 2
				if calval == 2:
					modeval = 2
			elif flg_alert == 1:
				data = {"jsonrpc":"2.0","method":"set_state", "params":{"screen":screenval,"bms":bmsval,"vol":volval, "lang":langval,"audio":audioval,"mode":modeval,"alert":alertval,"room_number":"001"}, "id":idval}
			elif flg_syserr == 1:
				data = {"jsonrpc":"2.0","method":"set_state", "params":{"screen":screenval,"bms":bmsval,"vol":volval, "lang":langval,"audio":audioval,"mode":modeval,"syserr":syserrval,"room_number":"001"}, "id":idval}
			else:
				data = {"jsonrpc":"2.0","method":"set_state", "params":{"screen":screenval,"bms":bmsval,"vol":volval, "lang":langval,"audio":audioval,"mode":modeval,"room_number":"001"},"id":idval}
				#data = {"jsonrpc":"2.0","method":"set_state","params":{"screen":1,"bms":3,"vol":3,"lang":1,"audio":1,"mode":2,"room_number":"001"},"id":2}

			json_data = json.dumps(data)
			encoded_data = json_data.encode()
			ser.write(encoded_data)
			ser.write(b'\n')
		      # print("sent: ", json_data)

			idval += 1
			if idval >=1000000:
				idval = 2
		except Exception as e:
			print("Error sending data",e)

serPort = serialInit()
serPort.timeout = 0
writeData(serPort)