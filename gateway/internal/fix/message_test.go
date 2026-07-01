package fix

import "testing"

func TestDecoderHandlesFragmentedAndCoalescedMessages(t *testing.T) {
	first := Encode("FIX.4.4", Field{35, "0"}, Field{49, "CLIENT"})
	second := Encode("FIX.4.4", Field{35, "A"}, Field{49, "CLIENT"})
	decoder := Decoder{}

	messages, err := decoder.Write(first[:5])
	if err != nil || len(messages) != 0 {
		t.Fatalf("partial write = %d messages, %v", len(messages), err)
	}
	messages, err = decoder.Write(append(first[5:], second...))
	if err != nil {
		t.Fatal(err)
	}
	if len(messages) != 2 || messages[0][35] != "0" || messages[1][35] != "A" {
		t.Fatalf("unexpected messages: %#v", messages)
	}
}

func TestDecoderRejectsBadChecksum(t *testing.T) {
	message := Encode("FIX.4.4", Field{35, "0"})
	message[len(message)-3] = '9'
	decoder := Decoder{}
	if _, err := decoder.Write(message); err == nil {
		t.Fatal("expected checksum error")
	}
}
