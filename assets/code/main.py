import time
import network
import socket
import machine
import ubinascii
import ili394x
from machine import Pin, SPI
import tt14
import cexample 

#Initialize TFT display
spi = SPI(1,baudrate=40000000,miso=Pin(12), mosi=Pin(11), sck=Pin(14))
display = ili394x.ILI9341(spi, cs=Pin(13), dc=Pin(5), rst=Pin(4),w=320,h=240,r=0)


#Opening Messege
display.erase()
text = 'Hello'
display.set_font(tt14)
display.set_pos(90,145)
display.print(text)

#Wifi Name and Password
#CHANGE THIS TO YOUR ACCESS POINT
ssid = ""
password = ""

#Define peripheral pins
sound_sensor_pin = Pin(1,Pin.IN)
led = machine.Pin("LED", machine.Pin.OUT)
bhadra_switch_pin = Pin(6,Pin.IN)
eric_switch_pin = Pin(8,Pin.IN)

#Initialise/Declare variables
eric_color = 0 #0 = red, 1 = green
bhadra_color = 0
red = "#eb4a4a;\">"
green = "#69df4e;\">"

#Read html file
html = open('page.html','r').read()

#Connect to wifi
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(ssid, password)

print(ubinascii.hexlify(wlan.config('mac'),':').decode())
print("Pico MAC:", wlan.config("mac"))
print()

#Wait for connection
max_wait = 5
while max_wait > 0:
    if wlan.status() < 0 or wlan.status() >= 3:
        break
    max_wait -= 1
    print("waiting...status=", wlan.status())
    time.sleep(1)

if wlan.status() != 3:
    print(wlan.status())
    raise RuntimeError("network connection failed :(")
else:
    print("connected!")
    status = wlan.ifconfig()
    print("assigned ip = " + status[0])

 # Create a socket and make a HTTP request
addr = socket.getaddrinfo(status[0], 80)[0][-1]
s = socket.socket()
s.bind(addr)
s.listen(1)



while True:
    response = html  
        
    try:
        cl, addr = s.accept()
        print("client connected from", addr)        
        request = cl.recv(1024)                 #find webpage URL
        request = str(request)                  #bytes to str
        print(request)
        print()        
        submit = request.find('?text')          #Submit button pressed
        check_in = request.find('?checkin')     #Check In Button Pressed
        play_audio = request.find('?Audio1')    #Audio 1 button pressed
        try:
            request = request.split()[1]
        except IndexError:
            pass
        text = request[7:]                      #Get entered text
        text = (text.replace("%20"," ")).lower() #ASCII decoding 
        print("submit is: ", submit)
        print("checkin is: ", check_in)
        print("Request is: ", request)
        if submit == 7:
            # SUBMIT BUTTON REQUEST RESPONSE: Display message on TFT
            print("submit button event!")
            display.erase()
            display.set_pos(0,0)
            display.print(text)
        
        if check_in == 7:
            # CHECK IN BUTTON REQUEST RESPONSE
            #Read switches 
            bhadra_switch = bhadra_switch_pin.value()
            eric_switch = eric_switch_pin.value()

            #Check if alarm is on and update webpage
            if (sound_sensor_pin.value() == 1):
                html = html.replace("class=\"alarmoff\"","class=\"alarmon\"")
                html = html.replace("Alarm Off","Alarm On")
                led.on()
        
            else:
                html = html.replace("class=\"alarmon\"","class=\"alarmoff\"")
                html = html.replace("Alarm On","Alarm Off")
                led.off()

            #check if switch states have changed and update background color on webpage if it has
            if eric_switch == 1 and eric_color == 0:
                eric_color = 1
                html = html.replace(red+"E",green+"E")
            elif eric_switch == 0 and eric_color == 1:
                eric_color = 0
                html = html.replace(green+"E",red+"E")
            if bhadra_switch == 1 and bhadra_color == 0:
                bhadra_color = 1
                html = html.replace(red+"B",green+"B")
            elif bhadra_switch == 0 and bhadra_color == 1:
                bhadra_color = 0
                html = html.replace(green+"B",red+"B")
                
        if play_audio == 7:
            # AUDIO BUTTON REQUEST RESPONSE
            cexample.add_ints(5,5)

            
            
        
        
        #request = str(request)
        
        cl.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
        cl.send(html)
        cl.close()
        
    except OSError as e:
        cl.close()
        print("connection closed")
