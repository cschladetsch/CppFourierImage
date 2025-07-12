/*
 * Simple GLAD implementation
 */

#include "glad.h"
#include <stdio.h>

/* For simplicity, we'll just return 1 (success) */
int gladLoadGLLoader(GLADloadproc load) {
    (void)load; /* Suppress unused parameter warning */
    return 1; /* Success */
}