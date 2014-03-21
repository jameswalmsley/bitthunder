# DocThunder - A Doxygen replacement

DocThunder is a doxygen replacement based on docurium from github.


# Custom Tags

  *	@public	 - This API is part of the public API. (You may use it).
  *	@private - This is an internal API, documented for interest only. Code using this may break.
  * @kernel  - Specifies the API is only accessible to objects linked to the kernel directly.
  *	@future	 - A note on changes to this API coming/desired in the future.
  * @warn	 - A special warning note. Docthunder will place this in a nice stylised bubble.

# Param Tags

The typical doxygen syntax is:

    @param Name Description

In DocThunder it is:

    @param[specifiers] Name Description

Either form is supported.

## Param specifiers.

Multiple specifiers can be used, each separated by a ','.

  * in	   - This paramater is an input.
  * out	   - This parameter is output (via a pointer).
  * inout  - This parameter is an input and output (through a pointer).
  * opt	   - This parameter is optional, and may be NULL if not used.
