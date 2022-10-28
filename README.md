# File Transfer

Server initiates with a port number as a parameter. It will wait for a connection from a client. Then afterwards it will receive the file name and the contents of the file until done. After client connection ends, the server will go back to waiting for a client. 

Client initiates with ip address and port number (of the server) as parameters. Assuming connection, it will ask the user for a file name. If the file exists it will send the name and the file's contents over to the server one byte at a time using TCP conection. Enter DONE to terminate the client. 
