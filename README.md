# AntiVape
Block vape v4 injection, vape lite coming soon:tm:


How it works:

Hooking JVMTI class file load hook
  -> check if class name starts with a/ (vape package name)
    -> if yes, replace content by random thing, that will make the game crash
