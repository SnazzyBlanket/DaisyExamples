# OscPocketD - XML to MIDI file (smf)

import xml.etree.ElementTree as ET



# construct Variable Length Value and add it to MIDI data buffer

def addVLV(mtime, buff):

	vtime = [0, 0, 0, 0]
	vtime[0] = (mtime >> 21) & 0x7f
	vtime[1] = (mtime >> 14) & 0x7f
	vtime[2] = (mtime >> 7) & 0x7f
	vtime[3] = (mtime >> 0) & 0x7f

	vflag = 0
	for i in range(0, 3):
		if vtime[i] != 0:
			vflag = 1
		if vflag != 0:
			vtime[i] = vtime[i] | 0x80

	for i in range(0, 4):
		if vtime[i] >= 0x80 or i == 3:
			buff.append(vtime[i])



# start of program

root = ET.parse('opd.xml').getroot()
tag = root.tag
attributes = root.attrib
upb = int(attributes.get("upb")) # units per beat
print("UPB=", upb)

f = open("opd.mid", "wb")

# header_chunk 
f.write( b"MThd")
f.write( (6).to_bytes(4, byteorder='big') ) # header length
f.write( (1).to_bytes(2, byteorder='big') ) # format: MIDI type 1
f.write( (7).to_bytes(2, byteorder='big') ) # tracks
f.write( (upb).to_bytes(2, byteorder='big') ) # ticks/division

cntChannel = 0 # MIDI track

# tracks

for track in root.findall('track'):

	attributes = track.attrib
	tracknumber = int(attributes.get('number'))
	print(" track", tracknumber)

	# keep a list of notes for the track
	notes = [] # track
	mtime = 0

	# go through all seq's for this track and add them

	for seq in track.findall('seq'):

		attributes = seq.attrib
		number = int(attributes.get('number'))
		print("  seq", number)

		# go through all notes for this seq

		for note in seq.findall("note"):
			pitch = int(note.find('pitch').text)
			length = int(note.find('length').text)
			notes.append([pitch, length])

	# go through all notes and construct MIDI data

	mtime = 0 # time pointer
	buff = [] # list of MIDI data

	if len(notes) > 0:

		for note in notes:

			# track_event = <v_time> + <midi_event>

			if (note[0] > 0): # the note is not a rest

				# note on

				if (tracknumber != 10): # not a drum track

					addVLV(mtime, buff)

					# midi event: NOTE ON: 9n, pitch, velocity
					buff.append( 144 + cntChannel)
					buff.append( note[0])
					buff.append( 100)

					mtime = note[1] # relative time

					# note off

					addVLV(mtime, buff)

					# midi event: NOTE ON WITH ZERO VELOCITY: 9n, pitch, velocity
					buff.append( 144 + cntChannel)
					buff.append( note[0])
					buff.append( 0)

					mtime = 0

				else:

					# drum track

					if (note[0] & 0b00000001): # bass drum
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(36)
						buff.append(100)
					if (note[0] & 0b00000010): # snare drum
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(38)
						buff.append(100)
					if (note[0] & 0b00000100): # hihat open
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(46)
						buff.append(100)
					if (note[0] & 0b00001000): # hihat closed
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(42)
						buff.append(100)
					if (note[0] & 0b00010000): # crash
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(49)
						buff.append(100)
					if (note[0] & 0b00100000): # clap
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(39)
						buff.append(100)
					
					mtime = note[1] # relative time
					#mtime = int(upb / 4)

					if (note[0] & 0b00000001): # bass drum
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(36)
						buff.append(0)
					if (note[0] & 0b00000010): # snare drum
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(38)
						buff.append(0)
					if (note[0] & 0b00000100): # hihat open
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(46)
						buff.append(0)
					if (note[0] & 0b00001000): # hihat closed
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(42)
						buff.append(0)
					if (note[0] & 0b00010000): # crash
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(49)
						buff.append(0)
					if (note[0] & 0b00100000): # clap
						addVLV(mtime, buff)
						buff.append(144 + cntChannel)
						buff.append(39)
						buff.append(0)

					mtime = 0

			else: # pitch == 0
				# rest
				mtime = mtime + note[1]

		if (len(buff) == 0):

			# import into DAW doesn't like tracks of len = 0, so add dummy note

			addVLV(0, buff)

			# midi event: NOTE ON: 9n, pitch, velocity
			buff.append( 144 + cntChannel)
			buff.append( 60)
			buff.append( 100)

			mtime = upb

			# note off

			addVLV(mtime, buff)

			# midi event: NOTE ON WITH ZERO VELOCITY: 9n, pitch, velocity
			buff.append( 144 + cntChannel)
			buff.append( 60)
			buff.append( 0)

		# end of track chunk: FF 2F 00 End of Track

		addVLV(mtime, buff)

		buff.append(0xFF)
		buff.append(0x2F)
		buff.append(0x00)

		# write the track to a file

		if len(buff) > 0:

			# track_chunk

			f.write( b"MTrk")
			f.write( (len(buff)).to_bytes(4, byteorder='big') ) # len, 4 bytes
			binary_format = bytearray(buff)
			f.write(binary_format)

	cntChannel = cntChannel + 1

f.close()