[Nibblo]

TODO :
[1] Implementasi mitigasi millis() integer overflow
inline bool isTimePassed(unsigned long now, unsigned long last, unsigned long interval) {
return (long)(now - last) > (long)interval;
}

[2] Perbaiki alert, pisahkan tiap CD

[3] Comment
