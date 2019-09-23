-- init.lua

print("node init")

NIMROD_PORT = 54011
NIMROD_IP = "192.168.1.199"   -- connect to the nimrod host
MY_NAME = "MCU000"  -- default to 000 - nimrod will allocate a unique number for us
MSG_LENGTH = 20
INPUT_1_PIN = 5
INPUT_2_PIN = 6
INPUT_3_PIN = 7
INPUT_4_PIN = 8
OUTPUT_LED_PIN = 0
OUTPUT_1_PIN = 1
OUTPUT_2_PIN = 2
OUTPUT_3_PIN = 3
OUTPUT_4_PIN = 4
OUTPUT_ON = 1
OUTPUT_OFF = 0
bConnectSocket = 0
bWaitingForSocketSent = 0
iLedState = 0
iConnected = -1
iSecSinceLastSend = 0
iRssi = 0
iLedTimer = 20
iOutput1Timer = -1
iOutput2Timer = -1
iOutput3Timer = -1
iOutput4Timer = -1
iOutput1Period = 10   -- seconds
iOutput2Period = 30   -- seconds
iOutput3Period = 30   -- seconds
iOutput4Period = 30   -- seconds
iOutput1State = OUTPUT_OFF
iOutput2State = OUTPUT_OFF
iOutput3State = OUTPUT_OFF
iOutput4State = OUTPUT_OFF


gpio.mode( OUTPUT_LED_PIN, gpio.OUTPUT )
gpio.mode( OUTPUT_1_PIN, gpio.OUTPUT )
gpio.mode( OUTPUT_2_PIN, gpio.OUTPUT )
gpio.mode( OUTPUT_3_PIN, gpio.OUTPUT )
gpio.mode( OUTPUT_4_PIN, gpio.OUTPUT )

gpio.mode( INPUT_1_PIN, gpio.INT, gpio.PULLUP )
gpio.mode( INPUT_2_PIN, gpio.INT, gpio.PULLUP )
gpio.mode( INPUT_3_PIN, gpio.INT, gpio.PULLUP )
gpio.mode( INPUT_4_PIN, gpio.INT, gpio.PULLUP )

-- led pin state is inverted
gpio.write( OUTPUT_LED_PIN, OUTPUT_OFF )
gpio.write( OUTPUT_1_PIN, iOutput1State )
gpio.write( OUTPUT_2_PIN, iOutput2State )
gpio.write( OUTPUT_3_PIN, iOutput3State )
gpio.write( OUTPUT_4_PIN, iOutput4State )



function enduser_connected()
  print( "enduser_setup is connected" )
    
end

-- run end user setup
enduser_setup.start( enduser_connected() )


-- connect the wifi
wifi.setmode( wifi.STATION )
-- dont set the wifi config if using enduser_setup as this prevents the valid config being saved to flash
--wifi.sta.config( { ssid="foobar", pwd=nil, save=true } )
wifi.sta.connect()


function socketSendTimeout()
  if bWaitingForSocketSent == 1 then
    print( "skt send tout" )
    
    srv1:close()
    bConnectSocket = 1
  end
end

function socket_send( msg )
  while string.len(msg) < MSG_LENGTH do
    msg = msg .. "."
  end

  print( "skt send: " .. msg )
    
  srv1:send( msg, socket_sent_cb )
  
  iSecSinceLastSend = 0
  bWaitingForSocketSent = 1
  tmr.alarm( 6, 500, tmr.ALARM_SINGLE, socketSendTimeout )
end

function button1Changed()
  return buttonChanged(INPUT_1_PIN)
end

function button2Changed()
  return buttonChanged(INPUT_2_PIN)
end

function button3Changed()
  return buttonChanged(INPUT_3_PIN)
end

function button4Changed()
  return buttonChanged(INPUT_4_PIN)
end

function buttonChanged( pin )
  local val = gpio.read(pin)
  
  iLedTimer = 20
  print('Pin ' .. pin .. ' val now '.. val )
  
  if val ~= 0 then
    -- do nothing
    
  elseif pin == INPUT_1_PIN then
    -- button 1 is pressed
    if iConnected > 0 then
      print( "send CLK1" )
      socket_send( MY_NAME .. "CLK1" )
    else
      if iOutput1State == OUTPUT_OFF then
        activate_output2( 1, iOutput1Period )
      else
        activate_output2( 1, 0 )
      end
    end
    
  elseif pin == INPUT_2_PIN then
    -- button 2 is pressed
    if iConnected > 0 then
      print( "send CLK2" )
      socket_send( MY_NAME .. "CLK2" )
    else
      if iOutput2State == OUTPUT_OFF then
        activate_output2( 2, iOutput2Period )
      else
        activate_output2( 2, 0 )
      end
    end
    
  elseif pin == INPUT_3_PIN then
    -- button 3 is pressed
    if iConnected > 0 then
      print( "send CLK3" )
      socket_send( MY_NAME .. "CLK3" )
    else
      if iOutput3State == OUTPUT_OFF then
        activate_output2( 3, iOutput3Period )
      else
        activate_output2( 3, 0 )
      end
    end
    
  elseif pin == INPUT_4_PIN then
    -- button 4 is pressed
    if iConnected > 0 then
      print( "send CLK4" )
      socket_send( MY_NAME .. "CLK4" )
    else
      if iOutput4State == OUTPUT_OFF then
        activate_output2( 4, iOutput4Period )
      else
        activate_output2( 4, 0 )
      end
    end
    
  end 
end

-- callbacks for button debounce
function button1_cb()
  tmr.alarm( 2, 50, tmr.ALARM_SINGLE, button1Changed )
end

function button2_cb()
  tmr.alarm( 3, 50, tmr.ALARM_SINGLE, button2Changed )
end

function button3_cb()
  tmr.alarm( 4, 50, tmr.ALARM_SINGLE, button3Changed )
end

function button4_cb()
  tmr.alarm( 5, 50, tmr.ALARM_SINGLE, button4Changed )
end



function wifi_disconnect_cb()
  wifi.sta.connect()
end


function timer_cb()
	if wifi.sta.getip() == nil then
	  if iConnected < 0 then
	    iConnected = 0
      print( "IP unavaiable, Waiting..." )
    end
    
  elseif iConnected == 0 then
    iConnected = 1
    iRssi = wifi.sta.getrssi()

    print( "IP is " .. wifi.sta.getip() )
    print( "RSSI is " .. iRssi )

    print( "connect skt" )
    srv1:connect( NIMROD_PORT, NIMROD_IP )
    
	end

	-- flash the led
	if iConnected <= 0 then
    -- wifi is disconnected
    if iLedState == 0 then
      gpio.write( OUTPUT_LED_PIN, OUTPUT_OFF )
    else
      gpio.write( OUTPUT_LED_PIN, OUTPUT_ON )
    end
	else
	  -- wifi is connected
	  if iLedTimer == 0 then
	    -- stop blinking the led after 30 seconds
      gpio.write( OUTPUT_LED_PIN, OUTPUT_ON )  
    elseif iLedState == 0 or iLedState == 2 then
      gpio.write( OUTPUT_LED_PIN, OUTPUT_OFF )
    else
      gpio.write( OUTPUT_LED_PIN, OUTPUT_ON )
    end
    
    -- check wifi is still connected
    if wifi.sta.status() ~= wifi.STA_GOTIP then
      print( "wifi is disconnected, state " .. wifi.sta.status() )
      iConnected = -1
    end
  end
  
  iLedState = iLedState + 1
  if iLedState >= 10 then
    -- every second
    iLedState = 0
    iSecSinceLastSend = iSecSinceLastSend + 1
    
    
    -- check wifi
    if iConnected == 1 then
      if wifi.sta.status() ~= wifi.STA_GOTIP then
        wifi.sta.disconnect( wifi_disconnect_cb )
        iConnected = -1
        iLedTimer = 20
      elseif iLedTimer > 0 then
        iLedTimer = iLedTimer - 1
      end
    end
    
    if iConnected == 1 and bConnectSocket == 1 then
      bConnectSocket = 0
      print( "reconnect skt" )
      srv1:connect( NIMROD_PORT, NIMROD_IP )
    elseif iConnected == 1 and iSecSinceLastSend >= 60 then
      -- send a ping
      socket_send( MY_NAME .. "PG" )
    end
    
    
    -- check output timers
    if iOutput1Timer > 0 then
      -- timer is active
      iOutput1Timer = iOutput1Timer - 1
      
      if iOutput1Timer == 0 then
        -- expired, turn off output
        print( "output 1 now off" )
        iOutput1Timer = -1;
        iOutput1State = OUTPUT_OFF
        gpio.write( OUTPUT_1_PIN, iOutput1State )
      end
    end
    
    if iOutput2Timer > 0 then
      -- timer is active
      iOutput2Timer = iOutput2Timer - 1
      
      if iOutput2Timer == 0 then
        -- expired, turn off output
        print( "output 2 now off" )
        iOutput2Timer = -1;
        iOutput2State = OUTPUT_OFF
        gpio.write( OUTPUT_2_PIN, iOutput2State )
      end
    end
    
    if iOutput3Timer > 0 then
      -- timer is active
      iOutput3Timer = iOutput3Timer - 1
      
      if iOutput3Timer == 0 then
        -- expired, turn off output
        print( "output 3 now off" )
        iOutput3Timer = -1;
        iOutput3State = OUTPUT_OFF
        gpio.write( OUTPUT_3_PIN, iOutput3State )
      end
    end
    
    if iOutput4Timer > 0 then
      -- timer is active
      iOutput4Timer = iOutput4Timer - 1
      
      if iOutput4Timer == 0 then
        -- expired, turn off output
        print( "output 4 now off" )
        iOutput4Timer = -1;
        iOutput4State = OUTPUT_OFF
        gpio.write( OUTPUT_4_PIN, iOutput4State )
      end
    end
    
  end
      
end


function activate_output( c )
  -- string index starts at 1 not 0
  -- 123456789012
  -- OKyxxxxx   output control
  -- PG000000   ping from nimrod
  -- NNMCUxxx   our new name
  if string.sub(c,1,2) == "OK" then
    local iOutput = tonumber(string.sub(c,3,3))
    local iPeriod = tonumber(string.sub(c,4,8))
    
    activate_output2( iOutput, iPeriod )
    
  elseif string.sub(c,1,2) == "NN" then
    MY_NAME = string.sub(c,3,8)
    
    print( "New name " .. MY_NAME )
    
  end
end

function activate_output2( iOutput, iPeriod )
  if iOutput ~= 0 then
    print( "Output " .. iOutput .. " on for " .. iPeriod .. " sec" )
  end
    
  if iOutput == 1 then
      
    if iPeriod == 0 then
      -- turn off
      iOutput1Timer = -1
      iOutput1State = OUTPUT_OFF
        
    else
      iOutput1Period = iPeriod
      iOutput1State = OUTPUT_ON
        
      if iOutput1Timer < 0 then
        iOutput1Timer = iOutput1Period
      else
        iOutput1Timer = iOutput1Timer + iOutput1Period
      end
    end

    gpio.write( OUTPUT_1_PIN, iOutput1State )
    print( "tmr 1 set to " .. iOutput1Timer )
    
  elseif iOutput == 2 then

    if iPeriod == 0 then
      -- turn off
      iOutput2Timer = -1
      iOutput2State = OUTPUT_OFF
        
    else
      iOutput2Period = iPeriod    
      iOutput2State = OUTPUT_ON
        
      if iOutput2Timer < 0 then
        iOutput2Timer = iOutput2Period
      else
        iOutput2Timer = iOutput2Timer + iOutput2Period
      end
    end

    gpio.write( OUTPUT_2_PIN, iOutput2State )
    print( "tmr 2 set to " .. iOutput2Timer )
      
  elseif iOutput == 3 then

    if iPeriod == 0 then
      -- turn off
      iOutput3Timer = -1
      iOutput3State = OUTPUT_OFF
        
    else
      iOutput3Period = iPeriod
      iOutput3State = OUTPUT_ON
        
      if iOutput3Timer < 0 then
        iOutput3Timer = iOutput3Period
      else
        iOutput3Timer = iOutput3Timer + iOutput3Period
      end
    end

    gpio.write( OUTPUT_3_PIN, iOutput3State )
    print( "tmr 3 set to " .. iOutput3Timer )
      
  elseif iOutput == 4 then
    if iPeriod == 0 then
      -- turn off
      iOutput4Timer = -1
      iOutput4State = OUTPUT_OFF
        
    else
      iOutput4Period = iPeriod    
      iOutput4State = OUTPUT_ON
        
      if iOutput4Timer < 0 then
        iOutput4Timer = iOutput4Period
      else
        iOutput4Timer = iOutput4Timer + iOutput4Period
      end
    end

    gpio.write( OUTPUT_4_PIN, iOutput4State )
    print( "tmr 4 set to " .. iOutput4Timer )
      
  end 
end

function socket_recv_cb( sck, c )
  print( "skt recv: " .. string.upper(c) )
  
  activate_output( c )
end

function socket_sent_cb()
  bWaitingForSocketSent = 0
  print("skt msg sent")
end

function socket_connected_cb( sck )
  print("skt connected")
  
  -- 01234567890123456789
  -- MCU001CLK1
  -- MCUxxxCIDyyyyyyyy
  socket_send( MY_NAME .. "CID" .. node.chipid() )
end

function socket_disconnected_cb( sck )
  print( "skt disconnected" );
  bConnectSocket = 1
end



-- have one socket for each button
--srv1 = net.createConnection( net.TCP, 0 )
tls.cert.verify(false)
srv1 = tls.createConnection()
srv1:on( "sent", socket_sent_cb )
srv1:on( "receive", socket_recv_cb )
srv1:on( "connection", socket_connected_cb )
srv1:on( "disconnection", socket_disconnected_cb )







tmr.alarm( 1, 100, tmr.ALARM_AUTO, timer_cb )

gpio.trig( INPUT_1_PIN, "both", button1_cb )
gpio.trig( INPUT_2_PIN, "both", button2_cb )
gpio.trig( INPUT_3_PIN, "both", button3_cb )
gpio.trig( INPUT_4_PIN, "both", button4_cb )


print("loaded")
