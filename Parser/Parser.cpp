#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

fstream file("example_new.ts", ios::binary | ios::in);
fstream PID136("PID136.mp2", ios::binary | ios::out);
fstream PID174("PID174.264", ios::binary | ios::out);

class Header
{
private:
	uint8_t PacketSize = 188;
	uint16_t PID = 0;
	uint8_t TSC = 0;
	uint8_t AFC = 0;
	uint8_t CC = 0;
	uint8_t SB = 0;
	uint8_t E = 0;
	uint8_t S = 0;
	uint8_t T = 0;

public:
	void Parser(char* buffer) {
		SB = *buffer;

		buffer++;

		E = ((*buffer >> 7) & 1);
		S = ((*buffer >> 6) & 1);
		T = ((*buffer >> 5) & 1);

		for (int i = 4; i >= 0; i--) {
			PID += ((*buffer >> i) & 1) * pow(2, (i + 8)); //poniewaz PID znajduje sie w dwoch bajtach, tu liczony jest bardziej znaczacy bajt 
		}

		buffer++;

		for (int i = 7; i >= 0; i--) {
			PID += ((*buffer >> i) & 1) * pow(2, i);  //mniej znaczacy bajt
		}

		buffer++;

		for (int i = 7; i >= 0; i--) {
			if (i > 5)
				TSC += ((*buffer >> i) & 1) * pow(2, i - 6);  //7 i 6 bit
			if (i == 5 or i == 4)
				AFC += ((*buffer >> i) & 1) * pow(2, i - 4);  //5 i 4 bit
			if (i < 4)
				CC += ((*buffer >> i) & 1) * pow(2, i);       //3,2,1,0 bit
		}
	}

	void Print() {
		cout << " TS: SB = " << static_cast<int>(SB);
		cout << " E = " << static_cast<int>(E); //wykrywa jedynke (maja przeznaczony jeden bit)
		cout << " S = " << static_cast<int>(S);
		cout << " T = " << static_cast<int>(T);
		cout << " PID = " << static_cast<int>(PID);
		cout << " TSC = " << static_cast<int>(TSC) << " AFC = " << static_cast<int>(AFC) << " CC = " << static_cast<int>(CC);
	}

	void Reset() {
		SB = 0;
		E = 0;
		S = 0;
		T = 0;
		PID = 0;
		TSC = 0;
		AFC = 0;
		CC = 0;
	}

	uint8_t getAFC() { return AFC; }
	uint8_t getS() { return S; }
	uint8_t getCC() { return CC; }
	uint8_t getPID() { return PID; }
};

class adaptationField
{
private:
	uint8_t AFL = 0;
	uint8_t DC = 0;
	uint8_t RA = 0;
	uint8_t SPi = 0;
	uint8_t PR = 0;
	uint8_t OR = 0;
	uint8_t SPf = 0;
	uint8_t TP = 0;
	uint8_t EX = 0;

	uint64_t program_clock_reference_base = 0;
	uint16_t program_clock_reference_extension = 0;

	uint64_t original_program_clock_reference_base = 0;
	uint16_t original_program_clock_reference_extension = 0;

	uint8_t splice_countdown = 0;

	uint8_t transport_private_data_length = 0;
	uint8_t private_data_byte = 0;

	uint8_t adaptation_field_extension_length = 0;
	uint8_t ltwFlag = 0;
	uint8_t piecewiseRateFlag = 0;
	uint8_t seamlessSpliceFlag = 0;

	uint8_t ltw_valid_flag = 0;
	uint16_t ltw_offset = 0;

	uint32_t piecewise_rate = 0;

	uint8_t splice_type = 0;
	uint32_t DTS_next_AU = 0;
	uint8_t marker_bit = 0;

	uint8_t stuffingCount = 0;
	uint8_t offset_value = 0;
public:
	void Parser(char* buffer) {
		AFL = *buffer;
		stuffingCount = AFL;

		buffer++;

		if (AFL > 0) {
			DC = ((*buffer >> 7) & 1);
			RA = ((*buffer >> 6) & 1);
			SPi = ((*buffer >> 5) & 1);
			PR = ((*buffer >> 4) & 1);
			OR = ((*buffer >> 3) & 1);
			SPf = ((*buffer >> 2) & 1);
			TP = ((*buffer >> 1) & 1);
			EX = ((*buffer >> 0) & 1);

			stuffingCount--;	//AFL zajmuje 1 bajt

			buffer++;

			if (PR) { //PCR
				program_clock_reference_base = 0;

				for (int i = 7; i >= 0; i--) {
					program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 25);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 17);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 9);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 1);
				}

				buffer++;

				program_clock_reference_base += ((*buffer >> 7) & 1) * pow(2, 1);

				program_clock_reference_extension = 0;

				program_clock_reference_extension += ((*buffer >> 0) & 1) * pow(2, 1);

				buffer++;

				for (int i = 7; i >= 0; i--) {
					program_clock_reference_extension += ((*buffer >> i) & 1) * pow(2, i);
				}

				stuffingCount -= 6; //PCR zajmuje 6 bajtow

				buffer++;

			}
			if (OR) { //OPCR
				original_program_clock_reference_base = 0;

				for (int i = 7; i >= 0; i--) {
					original_program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 25);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					original_program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 17);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					original_program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 9);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					original_program_clock_reference_base += ((*buffer >> i) & 1) * pow(2, i + 1);
				}

				buffer++;

				original_program_clock_reference_base += ((*buffer >> 7) & 1) * pow(2, 1);

				original_program_clock_reference_extension = 0;

				original_program_clock_reference_extension += ((*buffer >> 0) & 1) * pow(2, 1);

				buffer++;

				for (int i = 7; i >= 0; i--) {
					original_program_clock_reference_extension += ((*buffer >> i) & 1) * pow(2, i);
				}

				stuffingCount -= 6;

				buffer++;
			}

			if (SPi) {
				splice_countdown = 0;
				for (int i = 7; i >= 0; i--) {
					splice_countdown += ((*buffer >> i) & 1) * pow(2, i);
				}
				stuffingCount -= 1;

				buffer++;
			}

			if (TP) {
				transport_private_data_length = 0;
				for (int i = 7; i >= 0; i--) {
					transport_private_data_length += ((*buffer >> i) & 1) * pow(2, i);
				}

				buffer++;

				private_data_byte = 0;
				for (int i = transport_private_data_length; i >= 0; i--) {
					private_data_byte += ((*buffer >> i) & 1) * pow(2, i);
				}
				stuffingCount -= 2;
				
				buffer++;
			}

			if (EX) {
				adaptation_field_extension_length = 0;
				for (int i = 7; i >= 0; i--) {
					adaptation_field_extension_length += ((*buffer >> i) & 1) * pow(2, i);
				}

				buffer++;

				ltwFlag = ((*buffer >> 7) & 1);
				piecewiseRateFlag = ((*buffer >> 6) & 1);
				seamlessSpliceFlag = ((*buffer >> 5) & 1);

				if (seamlessSpliceFlag) {
					stuffingCount -= 5;
				}
				buffer++;

				if (ltwFlag) {
					ltw_valid_flag = 0;
					ltw_offset = 0;
					ltw_valid_flag = ((*buffer >> 7) & 1);
					for (int i = 6; i >= 0; i--) {
						ltw_offset += ((*buffer >> i) & 1) * pow(2, i + 8);
					}
					
					buffer++;

					for (int i = 7; i >= 0; i--) {
						ltw_offset += ((*buffer >> i) & 1) * pow(2, i);
					}

					buffer++;
					stuffingCount -= 2;
				}

				if (piecewiseRateFlag) {
					piecewise_rate = 0;
					for (int i = 5; i >= 0; i--) {
						piecewise_rate += ((*buffer >> i) & 1) * pow(2, i + 16);
					}

					buffer++;

					for (int i = 7; i >= 0; i--) {
						piecewise_rate += ((*buffer >> i) & 1) * pow(2, i + 8);
					}

					buffer++;

					for (int i = 7; i >= 0; i--) {
						piecewise_rate += ((*buffer >> i) & 1) * pow(2, i);
					}

					stuffingCount -= 3;

					buffer++;
				}

				if (seamlessSpliceFlag) {
					DTS_next_AU = 0;
					marker_bit = 0;
					splice_type = 0;

					for (int i = 7; i >= 4; i--) {
						splice_type += ((*buffer >> i) & 1) * pow(2, i);
					}

					for (int i = 3; i >= 1; i--) {
						DTS_next_AU += ((*buffer >> i) & 1) * pow(2, i + 28);
					}

					marker_bit += ((*buffer >> 1) & 1) * pow(2, 3);
					
					buffer++;

					for (int i = 7; i >= 0; i--) {
						DTS_next_AU += ((*buffer >> i) & 1) * pow(2, i + 22);
					}

					buffer++;

					for (int i = 7; i >= 1; i--) {
						DTS_next_AU += ((*buffer >> i) & 1) * pow(2, i + 14);
					}
					marker_bit += ((*buffer >> 1) & 1) * pow(2, 2);
					
					buffer++;

					for (int i = 7; i >= 0; i--) {
						DTS_next_AU += ((*buffer >> i) & 1) * pow(2, i + 7);
					}

					buffer++;

					for (int i = 7; i >= 1; i--) {
						DTS_next_AU += ((*buffer >> i) & 1) * pow(2, i - 1);
					}
					marker_bit += ((*buffer >> 1) & 1) * pow(2, 1);

					stuffingCount -= 5;
					
					buffer++;
				}
			}

		}
	}

	void Print() {
		cout << " AF: L = " << static_cast<int>(AFL);
		cout << " DC = " << static_cast<int>(DC);
		cout << " RA = " << static_cast<int>(RA);
		cout << " SPi = " << static_cast<int>(SPi);
		cout << " PR = " << static_cast<int>(PR);
		cout << " OR = " << static_cast<int>(OR);
		cout << " SPf = " << static_cast<int>(SPf);
		cout << " TP = " << static_cast<int>(TP);
		cout << " EX = " << static_cast<int>(EX);

		if (PR) {
			cout << " PCR = " << static_cast<int>(program_clock_reference_base * 300 + program_clock_reference_extension);
		}

		if (OR) {
			cout << " OPCR = " << static_cast<int>(original_program_clock_reference_base);
		}

		cout << " Stuffing = " << static_cast<int>(stuffingCount);
	}

	uint8_t getLength() { return AFL; }
};

class PES
{
public:
	uint8_t PacketSize = 188;
private:
	enum eStreamID : uint8_t
	{
		eSID_programStreamMap = 0xBC,
		eSID_paddingStream = 0xBE,
		eSID_privateStream2 = 0xBF,
		eSID_ECM = 0xF0,
		eSID_EMM = 0xF1,
		eSID_programStreamDirectory = 0xFF,
		eSID_DSMCCStream = 0xF2,
		eSID_ITUT_H222_1_Type_E = 0xF8
	};

	uint32_t PacketStartCodePrefix = 0;
	uint32_t PTS = 0;
	uint32_t DTS = 0;
	double timePTS = 0;
	double timeDTS = 0;
	uint8_t StreamId = 0;
	uint16_t PacketLength = 0;
	uint8_t headerSize = 0;

	uint8_t PESdataLength = 0;

	uint8_t PesScramblingControl = 0;
	uint8_t PesPriority = 0;
	uint8_t dataAlignmentIndicator = 0;
	uint8_t copyright = 0;
	uint8_t originalOrCopy = 0;

	uint8_t PTS_DTSflag = 0;
	uint8_t ESCRFlag = 0;
	uint8_t ESRateFlag = 0;
	uint8_t DSMTrickModeFlag = 0;
	uint8_t additionalCopyInfoFlag = 0;
	uint8_t PESCRCFlag = 0;
	uint8_t PESExtensionFlag = 0;
	uint8_t PESHeaderDataLength = 0;

	uint8_t stream_id_extension_flag = 0;
public:
	void Parser(char* buffer) {
		headerSize = 6;
		PacketStartCodePrefix = 0;

		for (int i = 7; i >= 0; i--) {
			PacketStartCodePrefix += ((*buffer >> i) & 1) * pow(2, (i + 16));
		}

		buffer++;

		for (int i = 7; i >= 0; i--) {
			PacketStartCodePrefix += ((*buffer >> i) & 1) * pow(2, (i + 8));
		}

		buffer++;

		for (int i = 7; i >= 0; i--) {
			PacketStartCodePrefix += ((*buffer >> i) & 1) * pow(2, (i));
		}

		buffer++;


		StreamId = 0;

		for (int i = 7; i >= 0; i--) {
			StreamId += ((*buffer >> i) & 1) * pow(2, (i));
		}

		buffer++;

		PacketLength = 0;

		for (int i = 7; i >= 0; i--) {
			PacketLength += ((*buffer >> i) & 1) * pow(2, (i + 8));
		}

		buffer++;

		for (int i = 7; i >= 0; i--) {
			PacketLength += ((*buffer >> i) & 1) * pow(2, (i));
		}

		if (StreamId != eSID_programStreamMap and StreamId != eSID_paddingStream and StreamId != eSID_privateStream2 and StreamId != eSID_ECM and
			StreamId != eSID_EMM and StreamId != eSID_programStreamDirectory and StreamId != eSID_DSMCCStream and StreamId != eSID_ITUT_H222_1_Type_E)
		{
			headerSize += 3;

			buffer++;

			PesScramblingControl = ((*buffer >> 5) & 1) * pow(2, (1));
			PesScramblingControl += ((*buffer >> 4) & 1);
			PesPriority = ((*buffer >> 3) & 1);
			dataAlignmentIndicator = ((*buffer >> 2) & 1);
			copyright = ((*buffer >> 1) & 1);
			originalOrCopy = ((*buffer >> 0) & 1);

			buffer++;

			PTS_DTSflag = ((*buffer >> 7) & 1) * pow(2, (1));
			PTS_DTSflag += ((*buffer >> 6) & 1);
			ESCRFlag = ((*buffer >> 5) & 1);
			ESRateFlag = ((*buffer >> 4) & 1);
			DSMTrickModeFlag = ((*buffer >> 3) & 1);
			additionalCopyInfoFlag = ((*buffer >> 2) & 1);
			PESCRCFlag = ((*buffer >> 1) & 1);
			PESExtensionFlag = ((*buffer >> 0) & 1);

			buffer++;

			PESHeaderDataLength = static_cast<uint8_t>(*buffer);

			if (PTS_DTSflag == 0b10) {
				headerSize += 5;
				buffer++;

				PTS = 0;
				for (int i = 3; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 28);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 22);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 14);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 7);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i - 1);
				}
			}
			else if (PTS_DTSflag == 0b11) {
				headerSize += 10;

				PTS = 0;
				DTS = 0;

				buffer++;

				for (int i = 3; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 28);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 22);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 14);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i + 7);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					PTS += ((*buffer >> i) & 1) * pow(2, i - 1);
				}

				buffer++;

				for (int i = 3; i >= 1; i--) {
					DTS += ((*buffer >> i) & 1) * pow(2, i + 28);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					DTS += ((*buffer >> i) & 1) * pow(2, i + 22);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					DTS += ((*buffer >> i) & 1) * pow(2, i + 14);
				}

				buffer++;

				for (int i = 7; i >= 0; i--) {
					DTS += ((*buffer >> i) & 1) * pow(2, i + 7);
				}

				buffer++;

				for (int i = 7; i >= 1; i--) {
					DTS += ((*buffer >> i) & 1) * pow(2, i - 1);
				}
			}
		}
	}

	void Print() {
		cout << " Started PES: PSCP = " << static_cast<int>(PacketStartCodePrefix);
		cout << " SID = " << static_cast<int>(StreamId);
		cout << " L = " << static_cast<int>(PacketLength);
		if (PTS_DTSflag == 0b10) {
			cout << " PTS = " << static_cast<int>(PTS);
			timePTS = PTS;
			timePTS = timePTS / 90000;
			cout << " (Time=" << timePTS << "s) ";
		}
		else if (PTS_DTSflag == 0b11) {
			cout << " PTS = " << static_cast<int>(PTS);
			timePTS = PTS;
			timePTS = timePTS / 90000;
			cout << " (Time=" << timePTS << "s)";

			cout << " DTS = " << static_cast<int>(DTS);
			timeDTS = DTS;
			timeDTS = timeDTS / 90000;
			cout << " (Time=" << timeDTS << "s) ";
		}
	}

	void save(char* buffer, uint8_t dataBytes, uint16_t PID)
	{
		if (PID == 136){
			PID136.write(buffer, dataBytes);
		}
		else if(PID == 174){
			PID174.write(buffer, dataBytes);
		}

	}

public:
	//PES packet header
	uint32_t getPacketStartCodePrefix() const { return PacketStartCodePrefix; }
	uint8_t getStreamId() const { return StreamId; }
	uint8_t getHeaderSize() const { return headerSize; }
	uint16_t getPacketLength() { return PacketLength; }
};

class Parser {
private:
	Header header;
	adaptationField AF;
	PES PES;
	int PESbytes = 0;
	int lastPID = 0;
	int nextAudioCC = 0;
	int nextVideoCC = 0;
	unsigned int packetCounter = 0;
public:
	void Parse(char* buffer) {
		uint8_t bufferOffset = 0;

		header.Reset();
		header.Parser(buffer);

		bufferOffset += 4;

		char* AFbuffer = buffer + bufferOffset;

		if (header.getS() and (header.getPID() == 136 or header.getPID() == 174))
		{
			if (PES.getHeaderSize() > 0) {
				cout << " Finished PES: PcktLen = " << PESbytes
					<< " HeadLen: " << static_cast<int>(PES.getHeaderSize())
					<< " DataLen: " << PESbytes - PES.getHeaderSize();
				PESbytes = 0;
			}
		}

		if (packetCounter != 0) cout << endl;
		cout << setfill('0') << setw(10) << packetCounter++;

		header.Print();

		if (header.getAFC() != 1){
			AF.Parser(AFbuffer);
			AF.Print();

			bufferOffset += AF.getLength() + 1;
		}

		char* PESbuffer = buffer + bufferOffset;

		if (header.getPID() == 136 and (header.getCC() == nextAudioCC) or (header.getPID() == 174 and (header.getCC() == nextVideoCC))) {
			if (header.getS()) {
				PES.Parser(PESbuffer);
				PES.save(PESbuffer + PES.getHeaderSize(), 188 - bufferOffset - PES.getHeaderSize(), header.getPID());
				PES.Print();
			}
			else if (header.getAFC() == 1) {
				PES.save(PESbuffer, 188 - bufferOffset, header.getPID());
				cout << " Continue ";
			}
			else {
				PES.save(PESbuffer, 188 - bufferOffset, header.getPID());
			}
		}
		else if(header.getPID() == 136 or header.getPID() == 174){
			cout << " PcktLost ";
		}
		
		if (header.getPID() == 136) {
			if (header.getCC() == 15) {
				nextAudioCC = 0;
			}
			else nextAudioCC = header.getCC() + 1;
		}

		if (header.getPID() == 174) {
			if (header.getCC() == 15) {
				nextVideoCC = 0;
			}
			else nextVideoCC = header.getCC() + 1;
		}

		if (header.getPID() == 136 or header.getPID() == 174) {
			PESbytes += 188 - bufferOffset;
		}

		lastPID = header.getPID();
	}
};


int main()
{
	unsigned int counter = 0;
	char c;
	char buffer[188];
	Parser parser;

	for (int i = 0; file.get(c); i++) {
		buffer[counter] = c;
		if (counter == 187) {
			counter = 0;
			parser.Parse(buffer);
		}
		else
		{
			counter++;
		}
	}

	file.close();
	PID136.close();
	PID174.close();

	return 0;
}