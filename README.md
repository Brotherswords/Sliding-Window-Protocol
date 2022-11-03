# Sliding Window Protocol

Client sends over a message using a window size of 10, after each ACKED packet, moves up window by 2. Server will accept and send ACKs or nothing if a packet is dropped. Packets are randomly dropped by the server and the Client appropraiately responds by resending all unACKed packets in the window.
