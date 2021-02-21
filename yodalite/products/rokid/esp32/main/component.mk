#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS := cmd 
COMPONENT_SRCDIRS :=. cmd
COMPONENT_EMBED_TXTFILES := play_err.mp3 awake.mp3 startup.mp3 server_root_cert.pem
