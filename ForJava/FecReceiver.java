/**********************************************************************************************************************\
                    OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description   : Optimized SMPTE 2022-1 FEC receiver
   Main Developer : David Fischer (david.fischer.ch@gmail.com)
   Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
   Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
                    Haute école du paysage, d'ingénierie et d'architecture @ Genève
                    Telecommunications Laboratory

\**********************************************************************************************************************/
/*
  This file is part of smpte2022lib.

  This project is free software: you can redistribute it and/or modify it under the terms of the EUPL v. 1.1 as provided
  by the European Commission. This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  See the European Union Public License for more details.

  You should have received a copy of the EUPL General Public License along with this project.
  If not, see he EUPL licence v1.1 is available in 22 languages:
      22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>

  Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git
*/

package smpte2022lib;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.TreeMap;

import smpte2022lib.FecPacket.Direction;

// *********************************************************************************************************************
// Optimized SMPTE 2022-1 FEC receiver
public class FecReceiver
{
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constants >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static String EX_FLUSHING  = "Currently flushing buffers";
	public static String EX_STARTUP   = "Current position still not initialized (startup state)";
	public static String EX_DIRECTION = "FEC packet direction is neither col nor row";
	public static String EX_VALID_RTP_MP2TS = "pPacket is not valid (expected RTP packet + MPEG2-TS payload)";
	public static String EX_VALID_RTP = "pPacket is not valid (expected RTP packet)";

	public static String EX_COL_OVERWRITE =
		"Another column FEC packet is already registered to protect media packet n°%s";
	public static String EX_ROW_OVERWRITE = "Another row FEC packet is already registered to protect media packet n°%s";

	public static String ER_SET_MISSING =
		"\nMessage : Unable to register missing media packet\nDetails :\n\n"+
		"-== wait structure ==-\n%s\n"+
		"-== media sequence numbers ==-\nComputed min : %s\nComputed max : %s\nActual value : %s\n"+
		"\nEnd of the bug report.\n";

	public static String ER_GET_COL_CASCADE =
		"FEC column cascade : Unable to compute sequence number of the media packet to recover";

	public static String ER_NULL_COL_CASCADE =
			"FEC column cascade : Unable to find linked entry in crosses buffer";

	public static String ER_GET_ROW_CASCADE =
			"FEC row cascade : Unable to compute sequence number of the media packet to recover";

	public static String ER_NULL_ROW_CASCADE = "FEC row cascade : Unable to find linked entry in crosses buffer";

	public static String ER_COL_MISMATCH = "Column FEC packet n°%s, expected n°%s";
	public static String ER_ROW_MISMATCH = "Row FEC packet n°%s, expected n°%s";
	public static String ER_MISSING_COUNT = "Missing count != 1";
	public static String EX_DELAY_SECONDS = "FIXME delay based on time interval not implemented";

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Data Types >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public enum DelayUnits { packets, seconds }

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Fields >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// Debugging
	public volatile String tag = "FecReceiver"; //. Logging tag

	// Media packets storage, medias[media seq] = media pkt
	protected TreeMap<Integer, RtpPacket> medias;
	protected boolean startup;  //. Indicate that actual position must be initialized
	protected boolean flushing; //. Indicate that a flush operation is actually running
	protected int     position; //. Actual position (sequence number) in the medias buffer

	// Link media packets to fec packets able to recover it, cross[media seq] = { col seq, row seq }
	protected TreeMap<Integer, FecCross> crosses;

	// Fec packets + related informations storage, col[sequence] = { fec pkt + infos }
	protected TreeMap<Integer, FecWait> cols, rows;
	protected int matrixL; //. Detected FEC matrix size (number of columns)
	protected int matrixD; //. Detected FEC matrix size (number of rows)

	// Output
	protected BufferedOutputStream outPayload; //. TODO
	protected ArrayList<RtpPacket> outMedias;  //. TODO

	// Settings
	protected int        delayValue = 100;                //. RTP buffer delay value
	protected DelayUnits delayUnits = DelayUnits.packets; //. RTP buffer delay units

	// Statistics about media (buffers and packets)
	protected int mediaReceived;        //. Received media packets counter
	protected int mediaRecovered;       //. Recovered media packets counter
	protected int mediaAbortedRecovery; //. Aborted media packet recovery counter
	protected int mediaOverwritten;     //. Overwritten media packets counter
	protected int mediaMissing;         //. Missing media packets counter
	protected int maxMedia;             //. Largest amount of stored elements in the medias buffer

	// Statistics about fec (buffers and packets)
	protected int colReceived; //. Received column fec packets counter
	protected int rowReceived; //. Received row fec packets counter
	protected int colDropped;  //. Dropped column fec packets counter
	protected int rowDropped;  //. Dropped row fec packets counter
	protected int maxCross;    //. Largest amount of stored elements in the crosses buffer
	protected int maxCol;      //. Largest amount of stored elements in the columns buffer
	protected int maxRow;      //. Largest amount of stored elements in the rows buffer

	// Statistics about lost medias
	protected TreeMap<Integer, Integer> lostogram = new TreeMap<Integer, Integer>();
	protected int lostogramCounter = 0;

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Constructors >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public FecReceiver(BufferedOutputStream pOutPayload, ArrayList<RtpPacket> pOutMedias)
	{
		if (pOutPayload == null && pOutMedias == null)
			throw new IllegalArgumentException("pOutPayload and pOutMedias are both null");
		medias     = new TreeMap<Integer, RtpPacket>();
		crosses    = new TreeMap<Integer, FecCross>();
		cols       = new TreeMap<Integer, FecWait>();
		rows       = new TreeMap<Integer, FecWait>();
		startup    = true;
		outPayload = pOutPayload;
		outMedias  = pOutMedias;
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Properties >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// Settings
	public double getCurrentDelay()
	{
		if (medias.isEmpty()) return 0;
		switch (delayUnits)
		{
		case packets: return medias.size();
		case seconds: return medias.lastEntry().getValue().getTime() - medias.firstEntry().getValue().getTime();
		default: return 0;
		}
	}

	public void setDelay(int pValue, DelayUnits pUnits)
	{
		if (pUnits == DelayUnits.seconds)
			throw new UnsupportedOperationException("FIXME output of media packets based on time delay");
		delayValue = pValue;
		delayUnits = pUnits;
	}

	public int        getDelayValue() { return delayValue; }
	public DelayUnits getDelayUnits() { return delayUnits; }

	// Statistics about media (buffers and packets)
	public int getMediaReceived()        { return mediaReceived;        }
	public int getMediaPosition()        { return position;             }
	public int getMediaBuffered()        { return medias.size();        }
	public int getMediaRecovered()       { return mediaRecovered;       }
	public int getMediaAbortedRecovery() { return mediaAbortedRecovery; }
	public int getMediaOverwritten()     { return mediaOverwritten;     }
	public int getMediaMissing()         { return mediaMissing;         }
	public int getMaxMediaBuffered()     { return maxMedia;             }

	// Statistics about fec (buffers, packets and matrix)
	public int getColReceived()      { return colReceived;    }
	public int getRowReceived()      { return rowReceived;    }
	public int getColDropped()       { return colDropped;     }
	public int getRowDropped()       { return rowDropped;     }
	public int getCrossBuffered()    { return crosses.size(); }
	public int getColBuffered()      { return cols.size();    }
	public int getRowBuffered()      { return rows.size();    }
	public int getMaxCrossBuffered() { return maxCross;       }
	public int getMaxColBuffered()   { return maxCol;         }
	public int getMaxRowBuffered()   { return maxRow;         }
	public int getFecMatrixL()       { return matrixL;        }
	public int getFecMatrixD()       { return matrixD;        }

	public TreeMap<Integer, Integer> getLostogram() { return lostogram; }

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// Put an incoming media packet ====================================================================================
	public void put(RtpPacket pPacket, boolean pOnlyMP2TS) throws Exception
	{
		if (flushing) throw new IllegalStateException(EX_FLUSHING);
		if (pOnlyMP2TS)
		{
			if (!pPacket.isValidMP2T()) throw new IllegalArgumentException(EX_VALID_RTP_MP2TS);
		}
		else
		{
			if (!pPacket.isValid()) throw new IllegalArgumentException(EX_VALID_RTP);
		}

		// Put the media packet into medias buffer
		if (medias.put(pPacket.sequence, pPacket) != null) mediaOverwritten++;
		if (medias.size() > maxMedia) maxMedia = medias.size();
		mediaReceived++;

		FecCross cross = crosses.get(pPacket.sequence);
		if (cross != null)
		{
			// Simulate the recovery of a media packet to update buffers and potentially start
			recoverMediaPacket(pPacket.sequence, cross, null); // a recovery cascade !
		}

		out(); // FIXME maybe better to call it from another thread
	}

	// Put an incoming FEC packet, the algorithm will do the following according to these scenarios : ===
	// [1] The fec packet is useless if none of the protected media packets is missing                ===
	// [2] Only on media packet missing, fec packet is able to recover it now !                       ===
	// [3] More than one media packet are missing, fec packet stored for future recovery              ===
	public void put(FecPacket pPacket)
	{
		if (flushing) throw new IllegalStateException(EX_FLUSHING);
		if (!pPacket.isValid()) throw new IllegalArgumentException("Invalid FEC packet");

		switch (pPacket.direction)
		{
		case COL: colReceived++; break;
		case ROW: rowReceived++; break;
		default: throw new IllegalArgumentException(EX_DIRECTION);
		}

		FecWait  wait  = new FecWait(pPacket);
		FecCross cross = null;
		int mediaLost  = 0;
		int mediaMin   = wait.snbase;
		int mediaMax   = (wait.snbase + wait.na * wait.offset) & RtpPacket.S_MASK;
		for (int mediaTest = mediaMin; mediaTest != mediaMax; mediaTest = (mediaTest + wait.offset) & RtpPacket.S_MASK)
		{
			// If media packet is not in the medias buffer (is missing)
			if (!medias.containsKey(mediaTest))
			{
				mediaLost = mediaTest;

				// TODO
				cross = crosses.get(mediaTest);
				if (cross == null)
				{
					cross = new FecCross();
					crosses.put(mediaTest, cross);
					if (crosses.size() > maxCross) maxCross = crosses.size();
				}
				// Register the fec packet able to recover the missing media packet
				switch (wait.direction)
				{
				case COL:
					if (cross.colSequence != -1)
					{
						String message = String.format(EX_COL_OVERWRITE, mediaLost);
						throw new IllegalStateException(message);
					}
					cross.colSequence = wait.sequence;
					break;
				case ROW:
					if (cross.rowSequence != -1)
					{
						String message = String.format(EX_ROW_OVERWRITE, mediaLost);
						throw new IllegalStateException(message);
					}
					cross.rowSequence = wait.sequence;
					break;
				default: throw new IllegalArgumentException(EX_DIRECTION);
				}
				// Updates the "waiting" fec packet
				try
				{
					wait.setMissing(mediaTest);
				}
				catch (Exception e)
				{
					throw new InternalError(String.format (ER_SET_MISSING, wait.toString(), mediaMin, mediaMax,
											mediaTest));
				}
			}
		}

		if (wait.getL() != 0) matrixL = wait.getL();
		if (wait.getD() != 0) matrixD = wait.getD();

		// [1] The fec packet is useless if none of the protected media packets is missing
		if (wait.missingCount == 0) return;

		// FIXME check if 10*delayValue is a good way to avoid removing early fec packets !
		// The fec packet is useless if it needs an already output'ed media packet to do recovery
		boolean drop = !ValidityWindow(wait.snbase, position, (position + 10*delayValue) & RtpPacket.S_MASK);

		switch (wait.direction)
		{
		case COL:
			if (drop) { colDropped++; return; }
			cols.put(wait.sequence, wait);
			if (cols.size() > maxCol) maxCol = cols.size();
			break;
		case ROW:
			if (drop) { rowDropped++; return; }
			rows.put(wait.sequence, wait);
			if (rows.size() > maxRow) maxRow = rows.size();
			break;
		}

		// [2] Only on media packet missing, fec packet is able to recover it now !
		if (wait.missingCount == 1)
		{
			recoverMediaPacket(mediaLost, cross, wait);
			out(); // FIXME maybe better to call it from another thread
		}
		// [3] More than one media packet are missing, fec packet stored for future recovery
	}

	// Flush all buffers and output media packets to the buffered output ===============================================
	public void flush() throws IOException
	{
		try
		{
			flushing = true;
			out();
			if (outPayload != null) outPayload.flush();
		}
		finally
		{
			flushing = false;
		}
	}

	// Remove FEC packets that are stored/waiting but useless ==========================================================
	public void cleanup()
	{
		if (flushing) throw new IllegalStateException(EX_FLUSHING);
		if (startup)  throw new IllegalStateException(EX_STARTUP);
		if (delayUnits != DelayUnits.packets)
			throw new IllegalArgumentException(EX_DELAY_SECONDS); // FIXME

		int start = position, end = (position + delayValue) & RtpPacket.S_MASK;

		List<Integer> mediasSequences = new ArrayList<Integer>();
		for (Integer mediaSequence : crosses.keySet())
		{
			mediasSequences.add(mediaSequence);
		}
		for (Integer mediaSequence : mediasSequences)
		{
			if (!ValidityWindow(mediaSequence, start, end))
			{
				FecCross cross = crosses.get(mediaSequence);
				cols.remove(cross.colSequence);
				rows.remove(cross.rowSequence);
				crosses.remove(mediaSequence);
			}
		}
	}

	protected void recoverMediaPacket(int pMediaSequence, FecCross pCross, FecWait pWait)
	{
		boolean recoveredByFec = pWait != null; // [2 or 3]

		// [1 or 2 or 3] Read and remove "cross" it from the buffer
		int colSequence = pCross.colSequence;
		int rowSequence = pCross.rowSequence;
		crosses.remove(pMediaSequence);

		// [2 or 3] Recover the missing media packet and remove any useless linked fec packet
		if (recoveredByFec)
		{
			if (pWait.missingCount != 1) throw new InternalError(ER_MISSING_COUNT);
			if (pWait.direction == Direction.COL && pWait.sequence != colSequence)
			{
				String message = String.format(ER_COL_MISMATCH, pWait.sequence, colSequence);
				throw new InternalError(message);
			}
			if (pWait.direction == Direction.ROW && pWait.sequence != rowSequence)
			{
				String message = String.format(ER_ROW_MISMATCH, pWait.sequence, rowSequence);
				throw new InternalError(message);
			}

			// Media packet recovery
			RtpPacket media = new RtpPacket();

			// 1 > Copy fec packet fields into the media packet
			media.sequence    = pMediaSequence;
			media.payloadType = pWait.payloadTypeRecovery;
			media.timestamp   = pWait.timestampRecovery;
			int payloadSize   = pWait.lengthRecovery;
			byte[] payload    = pWait.payloadRecovery;

			// 2 > recovered payload ^= all media packets linked to the fec packet
			boolean aborted = false;
			int mediaMin =  pWait.snbase;
			int mediaMax = (pWait.snbase + pWait.na * pWait.offset) & RtpPacket.S_MASK;
			for (int mediaTest = mediaMin; mediaTest != mediaMax;
					 mediaTest = (mediaTest + pWait.offset) & RtpPacket.S_MASK)
			{
				if (mediaTest == pMediaSequence) continue;

				RtpPacket friend = medias.get(mediaTest);

				// Unable to recover the media packet if any of the friend media packets is missing
				if (friend == null)
				{
					mediaAbortedRecovery++;
					aborted = true;
					break;
				}

				media.payloadType ^= friend.payloadType;
				media.timestamp   ^= friend.timestamp;
				payloadSize       ^= friend.getPayloadSize();
				for (int no = 0; no < Math.min(payload.length, friend.payload.length); no++)
				{
					payload[no] ^= friend.payload[no];
				}
			}

			// If the media packet is successfully recovered
			if (!aborted)
			{
				media.payload = Arrays.copyOfRange(payload, 0, payloadSize);

				mediaRecovered++;
				if (medias.put(media.sequence, media) != null) mediaOverwritten++;
				if (medias.size() > maxMedia) maxMedia = medias.size();

				switch (pWait.direction)
				{
				case COL: cols.remove(pWait.sequence); break;
				case ROW: rows.remove(pWait.sequence); break;
				}
			}
		}

		// [1 or 2 or 3] Check if a cascade effect happens ...
		FecWait waitCol = colSequence != -1 ? cols.get(colSequence) : null;
		FecWait waitRow = rowSequence != -1 ? rows.get(rowSequence) : null;
		if (waitCol != null) waitCol.setRecovered(pMediaSequence);
		if (waitRow != null) waitRow.setRecovered(pMediaSequence);

		if (waitCol != null)
		{
			if (waitCol.missingCount == 1)
			{
				// Cascade !
				int cascadeMediaSequence = waitCol.getMissingSequence(1);
				if (cascadeMediaSequence != -1)
				{
					FecCross cascadeCross = crosses.get(cascadeMediaSequence);
					if (cascadeCross != null)
					{
						recoverMediaPacket(cascadeMediaSequence, cascadeCross, waitCol);
					}
					else
					{
						throw new InternalError(
							"recoverMediaPacket("+pMediaSequence+", "+pCross+", "+pWait+"):\n"+
							ER_NULL_COL_CASCADE+"\n"+
							"media sequence : "+cascadeMediaSequence+"\n"+waitCol+"\n");
					}
				}
				else
				{
					throw new InternalError(ER_GET_COL_CASCADE+"\n"+waitCol+"\n");
				}
			}
		}

		if (waitRow != null)
		{
			if (waitRow.missingCount == 1)
			{
				// Cascade !
				int cascadeMediaSequence = waitRow.getMissingSequence(1);
				if (cascadeMediaSequence != -1)
				{
					FecCross cascadeCross = crosses.get(cascadeMediaSequence);
					if (cascadeCross != null)
					{
						recoverMediaPacket(cascadeMediaSequence, cascadeCross, waitRow);
					}
					else
					{
						throw new InternalError(ER_NULL_ROW_CASCADE+"\n"+
							"recoverMediaPacket("+pMediaSequence+", "+pCross+", "+pWait+"):\n"+
							"media sequence : "+cascadeMediaSequence+"\n"+waitRow+"\n");
					}
				}
				else
				{
					throw new InternalError(ER_GET_ROW_CASCADE+"\n"+waitRow+"\n");
				}
			}
		}
	}

	protected void out()
	{
		try
		{
			DelayUnits units = flushing ? DelayUnits.packets : delayUnits;
			int        value = flushing ? 0                  : delayValue;

			switch (units)
			{
			// Extract packets to output in order to keep a 'certain' amount of them in the buffer
			case packets: // based on buffer size
				while (medias.size() > value)
				{
					// Initialize or increment actual position (expected sequence number)
					position = (startup ? medias.firstKey() : position+1) & RtpPacket.S_MASK;
					startup  = false;

					RtpPacket media = medias.get(position);
					if (media != null)
					{
						// Update the histogram of the lost packets and reset the counter
						if (lostogram.containsKey(lostogramCounter))
							lostogram.put(lostogramCounter, lostogram.get(lostogramCounter) + 1);
						else
							lostogram.put(lostogramCounter, 1);
						lostogramCounter = 0;
						// Remove the media of the buffer and output it
						medias.remove(media.sequence);
						if (outPayload != null) outPayload.write(media.payload);
						if (outMedias  != null) outMedias.add(media);
					}
					else
					{
						// Increment the counters because another packet is missing ...
						mediaMissing++;
						lostogramCounter++;
					}

					// Remove any fec packet linked to current media packet
					FecCross cross = crosses.remove(position);
					if (cross != null)
					{
						if (cross.colSequence != -1) cols.remove(cross.colSequence);
						if (cross.rowSequence != -1) rows.remove(cross.rowSequence);
					}
				}
				break;

			// Extract packets to output in order to keep a 'certain' amount of them in the buffer
			case seconds: // based on time stamps
				throw new IllegalArgumentException(EX_DELAY_SECONDS); // FIXME
				/*double lastTime = medias.lastEntry().getValue().getTime();
				while (true)
				{
					// Get first media packet from medias buffer
					entry = medias.firstEntry();
					int mediaSequence = entry.getKey();

					// Check if it is necessary to output packet(s) (based on time interval)
					if (lastTime - entry.getValue().getTime() <= value)
					{
						if (lastTime - entry.getValue().getTime() <= 0)
						{
							System.out.println("last .time="+lastTime);
							System.out.println("first.time="+entry.getValue().getTime());
						}
						break;
					}

					// Compute expected media sequence number (so called position)
					if (startup)
					{
						// Initialize position
						position = mediaSequence;
						startup  = false;
					}else
					{
						// Increment position
						position = (position + 1) & RtpPacket.S_MASK;
					}

					// Check if a media packet is missing (not in the medias buffer)
					if (mediaSequence != position)
					{
						System.out.println("Missing media packet n°"+mediaSequence);
						mediaMissing++;
					}
					else
					{
						medias.remove(entry.getKey());
						if (output != null) output.write(entry.getValue().payload);
					}

					// Remove any fec packet linked to this media packet
					FecCross cross = crosses.remove(mediaSequence);
					if (cross != null)
					{
						if (cross.colSequence != -1) cols.remove(cross.colSequence);
						if (cross.rowSequence != -1) rows.remove(cross.rowSequence);
					}
				}
				break;*/
			default:
				System.out.println("Unknown delay units : "+delayUnits);
				break;
			}
		} catch (IOException e)
		{
			System.out.println("Unable to write to output : "+Utils.getException(e));
		}
	}

	public String toString()
	{
		String mReceived = String.format("%8d", getMediaReceived());
		String cReceived = String.format("%8d", getColReceived());
		String rReceived = String.format("%8d", getRowReceived());

		String mBuffered = String.format("%9d", getMediaBuffered());
		String cBuffered = String.format("%9d", getColBuffered());
		String rBuffered = String.format("%9d", getRowBuffered());
		String xBuffered = String.format("%9d", getCrossBuffered());

		String mMaximum = String.format("%8d", getMaxMediaBuffered());
		String cMaximum = String.format("%8d", getMaxColBuffered());
		String rMaximum = String.format("%8d", getMaxRowBuffered());
		String xMaximum = String.format("%8d", getMaxCrossBuffered());

		String cDropped = String.format("%8d", getColDropped());
		String rDropped = String.format("%8d", getRowDropped());

		int mPosition          = getMediaPosition();
		String mRecovered      = String.format("%9d",  getMediaRecovered());
		String mAborted        = String.format("%8d",  getMediaAbortedRecovery());
		String mOverwritten    = String.format("%12d", getMediaOverwritten());
		String mMissing        = String.format("%8d",  getMediaMissing());
		double     delayValue  = getCurrentDelay();
		DelayUnits delayUnits  = getDelayUnits();
		String     delayFormat = delayUnits == DelayUnits.packets ? "%.0f" : "%.2f";
		String     mDelay      = String.format(delayFormat, delayValue)+" "+delayUnits;

		return
			"tag : "+tag+"\n"+
			"Name  Received Buffered Maximum Dropped\n"+
			"Media "+mReceived+mBuffered+mMaximum+"\n"+
			"Col   "+cReceived+cBuffered+cMaximum+cDropped+"\n"+
			"Row   "+rReceived+rBuffered+rMaximum+rDropped+"\n"+
			"Cross "+"        "+xBuffered+xMaximum+"\n"+
			"FEC statistics, media packets :\n"+
			"Recovered Aborted Overwritten Missing\n"+
			mRecovered+mAborted+mOverwritten+mMissing+"\n"+
			"Current position (media sequence) : "+mPosition+"\n"+
			"Current delay (can be set) : "+mDelay+"\n"+
			"FEC matrix size (LxD) : "+matrixL+"x"+matrixD+" = "+(matrixL*matrixD)+" packets\n";
	}

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Static >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	public static InetSocketAddress computeColAddress(InetSocketAddress pMediaAddress)
	{
		if (pMediaAddress == null) return null;

		InetAddress address = pMediaAddress.getAddress();
		if (address.isMulticastAddress())
		{
			byte[] b = address.getAddress();
			b[3]++; if (b[3] == 0) { b[2]++; if (b[2] == 0) { b[1]++; if (b[1] == 0) b[0]++; } }
			try
			{
				address = InetAddress.getByAddress(b);
			}
			catch (UnknownHostException e) { return null; }
		}
		else
		{
			return null;
		}
		return new InetSocketAddress(address, pMediaAddress.getPort()+2);
	}

	public static InetSocketAddress computeRowAddress(InetSocketAddress pMediaAddress)
		throws UnknownHostException
	{
		return computeColAddress(computeColAddress(pMediaAddress));
	}

	protected static boolean ValidityWindow(int pCurrent, int pStart, int pEnd)
	{
		// Validity window [====] may be 'split' in two (0->end, start->2^16-1) :
		// 1) Linear : start=     6 end=9 :   0   1  2 3 4 5 [=======] 10 ... 65'533  65'534 65'535
		// 2) Split  : start=65'534 end=1 :  ======] 2 3 4 5  6 7 8 9  10 ... 65'533 [=============
		return (pEnd > pStart) ? (pCurrent >= pStart && pCurrent <= pEnd) :
								 (pCurrent <= pEnd   || pCurrent >= pStart);
	}

	public static int UnitTest() // FIXME put this in a real unit test
	{
		if (!RtpPacket.UnitTest())   return 1; // RtpPacket unit test must be successful
		if (FecWait.UnitTest() != 0) return 2; // FecWait   unit test must be successful

		ByteArrayOutputStream output;
		FecReceiver           receiver;

		// Testing validity window condition ---------------------------------------------------------------------------

		if (ValidityWindow(    0,     5, 10) != false) return 3;
		if (ValidityWindow(    5,     5, 10) != true)  return 4;
		if (ValidityWindow(    8,     5, 10) != true)  return 5;
		if (ValidityWindow(   10,     5, 10) != true)  return 6;
		if (ValidityWindow(   15,     5, 10) != false) return 7;
		if (ValidityWindow(    0, 65534,  2) != true)  return 8;
		if (ValidityWindow(    2, 65534,  2) != true)  return 9;
		if (ValidityWindow(    5, 65534,  2) != false) return 10;
		if (ValidityWindow(65534, 65534,  2) != true)  return 11;
		if (ValidityWindow(65535, 65534,  2) != true)  return 12;

		// Media packets are sorted by the buffer, so, it's time to test this feature ----------------------------------

		output   = new ByteArrayOutputStream();
		receiver = new FecReceiver(new BufferedOutputStream(output), null);
		receiver.setDelay(1024, DelayUnits.packets);

		List<Integer> source1   = new ArrayList<Integer>();
		String        expected1 = "";
		for (int i = 0; i < 1024; i++)
		{
			source1.add(i);
			expected1 += Integer.toString(i);
		}
		Collections.shuffle(source1);
		for (int i = 0; i < 1024; i++)
		{
			int sequence = source1.get(i);
			String payload = Integer.toString(sequence);
			try {
				receiver.put(RtpPacket.createMP2T(sequence, sequence*100, payload.getBytes()), true);
			} catch (Exception e) { return 13; }
		}
		try
		{
			receiver.flush();
			if (!expected1.equals(output.toString()))
				return 14;
		}
		catch (IOException e)
		{
			System.out.println(Utils.getException(e));
			return 15;
		}

		// Testing fec algorithm correctness ! -------------------------------------------------------------------------

		/* FIXME todo
		output   = new ByteArrayOutputStream();
		receiver = new FecReceiver(new BufferedOutputStream(output));
		receiver.setDelay(1024, DelayUnits.packets);

		byte L = 4, D = 5;
		RtpPacket[][] matrix = Utils.createRandomRtpPacketsMatrix(L, D, 50, 100);

		// Retrieve the first column of the matrix
		List<RtpPacket> packets = new ArrayList<RtpPacket>();
		for (int d = 0; d < D; d++)
		{
			/*for (int l = 0; l < L; l++)
			{/
				RtpPacket rtp = matrix[d][0];
				receiver.put(rtp);
				packets.add(rtp);
			/*}/
		}
		receiver.put(new FecPacket(Algorithm.XOR, Direction.COL, L, D, packets));

		System.out.println(receiver.toString());

		try
		{
			receiver.flush();
			//if (!expected1.equals(output.toString())) return false;
		}
		catch (IOException e)
		{
			System.out.println(Utils.getException(e));
			return false;
		}*/

		// Testing helpers ---------------------------------------------------------------------------------------------

		byte[] b;
		InetSocketAddress media, col, row;
		try
		{
			media = Utils.str2inet("");
			if (computeColAddress(media) != null || computeRowAddress(media) != null) return 16;

			// ---------------------------------------------------------------------------------------------------------

			media = new InetSocketAddress("239.232.0.222", 3000);

			col = computeColAddress(media);
			b   = col.getAddress().getAddress();
			if (b[0] != (byte)239 || b[1] != (byte)232 || b[2] != 0 || b[3] != (byte)223 || col.getPort() != 3002)
				return 17;

			row = computeRowAddress(media);
			b   = row.getAddress().getAddress();
			if (b[0] != (byte)239 || b[1] != (byte)232 || b[2] != 0 || b[3] != (byte)224 || row.getPort() != 3004)
				return 18;

			// ---------------------------------------------------------------------------------------------------------

			// FIXME I know that IPv4 addresses ending with 255 are ... ;-)
			media = new InetSocketAddress("239.232.255.255", 8000);

			col = computeColAddress(media);
			b   = col.getAddress().getAddress();
			if (b[0] != (byte)239 || b[1] != (byte)233 || b[2] != 0 || b[3] != 0 || col.getPort() != 8002)
				return 19;

			row = computeRowAddress(media);
			b   = row.getAddress().getAddress();
			if (b[0] != (byte)239 || b[1] != (byte)233 || b[2] != 0 || b[3] != 1 || row.getPort() != 8004)
				return 20;
		}
		catch (Exception e) { System.out.println (Utils.getException(e)); return 21; }

		return 0;
	}
}
