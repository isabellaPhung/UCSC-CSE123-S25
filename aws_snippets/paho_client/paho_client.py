import sys
import ssl
import time
import paho.mqtt.client as mqtt

import json

IoT_protocol_name = "x-amzn-mqtt-ca"

topic = "iotdevice/55/datas3"

def ssl_alpn():
    ssl_context = ssl.create_default_context()
    ssl_context.set_alpn_protocols([IoT_protocol_name])
    ssl_context.load_verify_locations(cafile=ca)
    ssl_context.load_cert_chain(certfile=cert, keyfile=private)
    return  ssl_context

def on_connect(mqttc, obj, flags, reason_code, properties):
    print("reason_code: " + str(reason_code))

msg_len = 0
cur_index = -1
def on_message(mqttc, obj, msg):
    #print(topic + ":")
    #print(msg.payload)
    global msg_len
    global cur_index
    json_msg = json.loads(msg.payload)
    if json_msg["id"] == "server" and json_msg["action"] == "length":
        msg_len = int(json_msg["length"])
        cur_index = 0
        print(f"Expectng {msg_len} tasks");
    elif json_msg["id"] == "server" and json_msg["action"] == "response":
        print(msg.payload)
        cur_index += 1
        if cur_index == msg_len:
            mqttc.unsubscribe(topic)
            time.sleep(1)
            mqttc.disconnect()
            mqttc.loop_stop()

def on_subscribe(mqttc, obj, mid, reason_code_list, properties):
    print("Subscribed: " + str(mid) + " " + str(reason_code_list))

def on_unsubscribe(mqttc, obj, mid, reason_code_list, properties):
    print("Unsubscribed: " + str(mid) + " " + str(reason_code_list))


if __name__ == '__main__':
    argc = len(sys.argv)
    if argc < 5:
        print(f"Usage: {sys.argv[0]} <aws endpoint> <ca cert> <device cert> <private key> [message json]")
        exit()

    aws_iot_endpoint = sys.argv[1]
    url = "https://{}".format(aws_iot_endpoint)
    ca = sys.argv[2]
    cert = sys.argv[3]
    private = sys.argv[4]
    if argc > 5:
        json_path = sys.argv[5]
        json_f = open(json_path, "rt")
        json_text = json_f.read()
    else:
        json_text = '{"message":"Hello world!"}'

    mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    mqttc.on_message = on_message
    mqttc.on_connect = on_connect
    mqttc.on_subscribe = on_subscribe
    mqttc.on_unsubscribe = on_unsubscribe

    ssl_context= ssl_alpn()
    mqttc.tls_set_context(context=ssl_context)

    mqttc.connect(aws_iot_endpoint, port=443)

    mqttc.loop_start()

    time.sleep(3)
    mqttc.subscribe(topic)
    while (cur_index != msg_len):
        mqttc.publish(topic, json_text)

        time.sleep(5)
        if msg_len == 0:
            print("Did not get the first message, retying")
        else:
            break
