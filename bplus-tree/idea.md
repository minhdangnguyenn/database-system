## To calculate the order of bplus tree
### Leaf node
max_keys = (PAGE_SIZE - header_size) / (key_size + value_size)

header = is_leaf(1) + num_keys(4) + next_leaf(4) = 9 bytes

max_keys = (1024 - 9) / (4 + 4)
         = 1015 / 8
         = 126 keys per leaf

order = 126 / 2 = 63

### Internal Node
header = is_leaf(1) + num_keys(4) = 5 bytes

max_keys = (1024 - 5) / (key_size + pointer_size)
         = 1019 / (4 + 4)
         = 1019 / 8
         = 127 keys per internal node

order = 127 / 2 = 63
