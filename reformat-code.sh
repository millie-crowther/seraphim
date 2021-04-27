for filename in $(find include src -regex ".*\.\(c\|cpp\|h\)$"); do
  indent -linux "$filename"
  rm "$filename"~
done
