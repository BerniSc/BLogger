[![Visits Badge](https://badges.pufler.dev/visits/BerniSc/BLogger)](https://badges.pufler.dev)

# BLogger
Small Project for learning/configuring and getting used to nvim and my new NVim Config. 

The Goal was to have a project that is a little bit more complex than the default "Hello World" Stuff and forces me to use some more advanced `concepts` in nvim, including managing the git-workflow, makefile auto-creation etc.
Still I wanted to have something usefull/reusable, so I decided on a small C++-Logger.

It should be a Header-Only Logger with support for:
- [ ] File-Logging
    - [x] Basic File-Logging
    - [ ] Customizable Logpath
    - [ ] Log-Rotation
    - [x] Platform-Indepentent
- [x] Console-Logging
- [ ] Composite-Logging (Console+File in one)
- [x] Threadsafety
- [x] Topic-based Logging and filtering
- [x] Level-based Logging and filtering
- [x] Conditional Logging
- [x] Logging custom "Classes/Messages" 
- [x] Flexible way of configuring/customizing the Logged Message
    - [x] Easily expandable via Decorators
- [x] Out of the Box support for basic Logger-Functionabilities
    - [x] Log Location of log-Statement
    - [x] Log Timestamp
    - [ ] Log Username
- [x] Central Management and Distribution of all Loggers
    - [x] Overall setting of Loglevel possible
    - [x] Custom Loglevel per Loggerinstance still possible
- [ ] Ideally maintainable and easily and intuitively usable, but out of experience.... :-) 


## The (Rough) Class-Architecture of BLogger

![Class-Architecture of BLogger](./doc/BLoggerClassStructure.png)
