void write(client_context_t *context, byte *data, int data_size) {
	if ((!context) || (!context->socket) || (!context->socket->connected())) {
		CLIENT_ERROR(context, "The socket is null! (or is closed)");
		return;
	}
	if (context->disconnect) {
		context->error_write = true;
		return;
	}
	if (context->error_write) {
		CLIENT_ERROR(context, "Abort write data since error_write.");
		return;
	}
	int write_size = context->socket->write(data, data_size);
	CLIENT_DEBUG(context, "Sending data of size %d", data_size);
	if (write_size != data_size) {
		context->error_write = true;
		context->socket->keepAlive(1, 1, 1);		// fast disconnected internally in 1 second.
		CLIENT_ERROR(context, "socket.write, data_size=%d, write_size=%d", data_size, write_size);
	}
}
