for filename in $(find include src -regex ".*\.\(c\|cpp\|h\)$"); do
  indent -i4 -br -ce -brf -bap -sob -npcs -npsl -l85 -ts4 -nlp "$filename"
  rm "$filename"~
done
