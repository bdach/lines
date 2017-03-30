#!/usr/bin/ruby
require 'socket'

puts 'Welcome to LINSW!'
time = Time.new
puts 'Current time: ' + time.inspect

socket = UDPSocket.new
socket.bind('', 48000)

while true do
	sleep 10
	time = Time.new
	socket.send time.inspect, 0, '192.168.143.172', 48000
end
