message = table.concat(arguments, " ") -- These are command line arguments
-- print(#arguments) -- print the # of arguments passed

InitNetworking() -- Initialize WS2

local sock = CreateTCPSocket(false) -- Create socket
if sock == nil then
    print("Lua could not create a socket")
    os.exit(1)
end

local sockaddr = CreateSocketAddress("127.0.0.1", false, 6009) -- Set socket address

if ConnectToSocket(sock, sockaddr) then -- Connect to socket
    print("Connected!")
else
    print("No connection!")
end

if WriteToSocket(sock, message) then -- Write to socket
    print("Sent message")
else
    print("Message failed to send")
end

CloseSocket(sock) -- Close socket
DestroyNetworking() -- Close WS2