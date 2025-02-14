#pragma once

// Enumeration to define actions to take when an I2C error occurs
enum class ErrorAction {
    STOP,      // Stop execution on error
    RETRY,     // Retry the operation
    CONTINUE   // Skip the failed operation and continue
};
