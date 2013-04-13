G:
cd "G:\DavidDev\Projet VLC-SMPTE\Release\"

FecGenerator.exe auto source=%1 dest="fec_rtp.mpg"
ErrorsGenerator.exe auto source="fec_rtp.mpg" dest="fec_rtp_errors.mpg"

del "fec_rtp.mpg"

FecDecoder.exe auto source="fec_rtp_errors.mpg" destRaw="raw.mpg" destDavid="david.mpg" destBrute="brute.mpg"

del "fec_rtp_errors.mpg"

pause