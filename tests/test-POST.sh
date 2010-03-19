export REQUEST_URI=/patient1234/laborders/3.xml
export REQUEST_METHOD=POST
export REMOTE_ADDR=127.0.0.1
export CONTENT_LENGTH=10

echo "1234567890" | ./hstore
