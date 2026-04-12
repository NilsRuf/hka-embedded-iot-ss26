# Here, we only add the subdirectory to our CMake files if we have actually enabled the sensor.
add_subdirectory_ifdef(CONFIG_GROVE_RANGER grove_ranger)
