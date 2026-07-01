package fix

import (
	"bytes"
	"errors"
	"fmt"
	"strconv"
)

const SOH byte = 0x01

var ErrNeedMoreData = errors.New("need more FIX data")

type Message map[int]string

func (message Message) Require(tag int) (string, error) {
	value, ok := message[tag]
	if !ok || value == "" {
		return "", fmt.Errorf("required FIX tag %d is missing", tag)
	}
	return value, nil
}

type Decoder struct {
	buffer []byte
}

func (decoder *Decoder) Write(data []byte) ([]Message, error) {
	decoder.buffer = append(decoder.buffer, data...)
	var messages []Message
	for len(decoder.buffer) != 0 {
		message, consumed, err := decodeOne(decoder.buffer)
		if errors.Is(err, ErrNeedMoreData) {
			return messages, nil
		}
		if err != nil {
			decoder.buffer = nil
			return messages, err
		}
		messages = append(messages, message)
		decoder.buffer = decoder.buffer[consumed:]
	}
	return messages, nil
}

func decodeOne(data []byte) (Message, int, error) {
	if !bytes.HasPrefix(data, []byte("8=")) {
		return nil, 0, errors.New("FIX message must start with tag 8")
	}
	beginEnd := bytes.IndexByte(data, SOH)
	if beginEnd < 0 {
		return nil, 0, ErrNeedMoreData
	}
	lengthStart := beginEnd + 1
	if len(data) < lengthStart+2 || string(data[lengthStart:lengthStart+2]) != "9=" {
		return nil, 0, errors.New("tag 9 must immediately follow tag 8")
	}
	lengthRelativeEnd := bytes.IndexByte(data[lengthStart:], SOH)
	if lengthRelativeEnd < 0 {
		return nil, 0, ErrNeedMoreData
	}
	lengthEnd := lengthStart + lengthRelativeEnd
	bodyLength, err := strconv.Atoi(string(data[lengthStart+2 : lengthEnd]))
	if err != nil || bodyLength < 0 || bodyLength > 1024*1024 {
		return nil, 0, errors.New("invalid FIX BodyLength")
	}
	bodyStart := lengthEnd + 1
	checksumStart := bodyStart + bodyLength
	messageEnd := checksumStart + 7
	if len(data) < messageEnd {
		return nil, 0, ErrNeedMoreData
	}
	if string(data[checksumStart:checksumStart+3]) != "10=" || data[messageEnd-1] != SOH {
		return nil, 0, errors.New("FIX BodyLength does not end at tag 10")
	}
	expected, err := strconv.Atoi(string(data[checksumStart+3 : checksumStart+6]))
	if err != nil || expected < 0 || expected > 255 {
		return nil, 0, errors.New("invalid FIX CheckSum")
	}
	actual := 0
	for _, value := range data[:checksumStart] {
		actual += int(value)
	}
	if actual%256 != expected {
		return nil, 0, errors.New("FIX CheckSum mismatch")
	}

	message := Message{}
	for _, field := range bytes.Split(data[:checksumStart], []byte{SOH}) {
		if len(field) == 0 {
			continue
		}
		parts := bytes.SplitN(field, []byte{'='}, 2)
		if len(parts) != 2 {
			return nil, 0, errors.New("malformed FIX field")
		}
		tag, err := strconv.Atoi(string(parts[0]))
		if err != nil {
			return nil, 0, errors.New("FIX tag is not numeric")
		}
		message[tag] = string(parts[1])
	}
	message[10] = string(data[checksumStart+3 : checksumStart+6])
	return message, messageEnd, nil
}

type Field struct {
	Tag   int
	Value string
}

func Encode(beginString string, bodyFields ...Field) []byte {
	var body bytes.Buffer
	for _, field := range bodyFields {
		fmt.Fprintf(&body, "%d=%s%c", field.Tag, field.Value, SOH)
	}
	prefix := []byte(fmt.Sprintf("8=%s%c9=%d%c", beginString, SOH, body.Len(), SOH))
	message := append(prefix, body.Bytes()...)
	checksum := 0
	for _, value := range message {
		checksum += int(value)
	}
	return append(message, []byte(fmt.Sprintf("10=%03d%c", checksum%256, SOH))...)
}
