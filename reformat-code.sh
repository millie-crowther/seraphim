for filename in $(find include src -regex ".*\.\(c\|cpp\|h\)$"); do
  clang-format "$filename" -i
done
