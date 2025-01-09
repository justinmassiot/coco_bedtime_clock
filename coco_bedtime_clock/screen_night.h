// pictures and animations can be built at: https://ledmatrix-editor.arduino.cc/
// more help at: https://docs.arduino.cc/tutorials/uno-r4-wifi/led-matrix/

const uint32_t anim_asleep[][4] = {
	{
		0x1009,
		0x800800,
		0x90010000,
		10000
	},
	{
		0x1009,
		0x1401400,
		0x90010000,
		1000
	}
};

byte moon[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
