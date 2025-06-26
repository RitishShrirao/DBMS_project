I have created the parent as book and child as author.
New functionality in pds_tester.c:-

    1) Backwards compatibility if writing child name of file as NULL
    2) STORE_LINKED : store new records for child entity.
    Usage : STORE_LINKED <child key> <expected status>
    3) SEARCH_LINKED : linear search over linked data.
    Usage : SEARCH_LINKED <child key> <expected IO>
    4) LINK : Link parent and child with keys.
    Usage : LINK <parent key> <child key> <expected status>

    5) SEARCH_LINKED_BY_PARENT : Find all children using parent key.
    Usage: SEARCH_LINKED_BY_PARENT <parent key> <num_children> <child_key_start>... <child_key_end>
    num_children = -1 if expected status is some error.

Other functionalities:

    Deletion of link after deletion of parent and checking existence of both parent and children before linking.
    Handle same parent and child keys multiple times during linking.

The code can be compiled with
gcc roll_number_book.c roll_number_author.c roll_number_pds.c bst.c pds_tester.c
and run with
./a.out testcase.in
