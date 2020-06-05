robocopy Game\Assets Executable\Assets\Unity /E /COPYALL /MIR
cd Executable
python convert-unity.py
cd ..
