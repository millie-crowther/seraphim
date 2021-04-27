for filename in $(find include src -regex ".*\.\(c\|cpp\)$"); do
  cppcheck --enable=all "$filename" || exit 1
done
