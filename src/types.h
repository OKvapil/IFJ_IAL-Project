/**
 * @file types.h
 * @brief Data types of IFJ26
 *
 * IFJ/IAL project - IFJ26 compiler
 *
 * @author Ondřej Kvapil (xkvapio00)
 */

#ifndef IFJ_IAL_PROJECT_TYPES_H
#define IFJ_IAL_PROJECT_TYPES_H

/**
 * @brief Type of value passed to functions
 */
typedef enum {
  TYPE_INT,
  TYPE_DOUBLE,
  TYPE_STRING,
  TYPE_UNIT,
  TYPE_BOOL,
  TYPE_UNKNOWN,
} DataType;
#endif // IFJ_IAL_PROJECT_TYPES_H
