#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

#include "input_parser.hpp"
#include "enclosure.hpp"
namespace {

// Runs the parser on a text, in a 100 x 100 enclosure. No files are needed.
std::vector<common::particle_record> parse(std::string const & text) {
  std::istringstream input{text};
  return common::parse_input_stream(input, common::enclosure{100.0, 100.0});
}

}
TEST(Parser, MalformedLine){
 auto const particles = parse("1 2 50 50 3 4\n"
                               "abc def\n"
                               "5 1 20 30 -1 -2\n");

  ASSERT_EQ(particles.size(), 2U);

  EXPECT_DOUBLE_EQ(particles.at(0).mass, 1.0);   
  EXPECT_DOUBLE_EQ(particles.at(0).id, 0);// line 1 comes first
  EXPECT_DOUBLE_EQ(particles.at(1).mass, 5.0);   // line 3 comes second
  EXPECT_DOUBLE_EQ(particles.at(1).radius, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).id, 2);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.getx(), 20.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.gety(), 30.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.getx(), -1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.gety(), -2.0);

}
TEST(Parser, IncompleteLastLine){
 auto const particles = parse("1 2 50 50 3 4\n"
                               
                               "5 1 20 30 -1 ");
  ASSERT_EQ(particles.size(), 1U);

  EXPECT_DOUBLE_EQ(particles.at(0).mass, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(0).radius, 2.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.getx(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.gety(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.getx(), 3.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.gety(), 4.0);
  EXPECT_DOUBLE_EQ(particles.at(0).id, 0);


}

TEST(Parser, EmptyFile){
     auto const particles = parse("");
EXPECT_TRUE(particles.empty());

}
TEST(Parser, NoTrailingNewline){
 auto const particles = parse("1 2 50 50 3 4\n"
                                "5 1 20 30 -1 -2"  
                               
                               );
  ASSERT_EQ(particles.size(), 2U);

  EXPECT_DOUBLE_EQ(particles.at(0).mass, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(0).radius, 2.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.getx(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.gety(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.getx(), 3.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.gety(), 4.0);
  EXPECT_DOUBLE_EQ(particles.at(0).id, 0);

  EXPECT_DOUBLE_EQ(particles.at(1).mass, 5.0);
  EXPECT_DOUBLE_EQ(particles.at(1).radius, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.getx(), 20.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.gety(), 30.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.getx(), -1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.gety(), -2.0);
  EXPECT_DOUBLE_EQ(particles.at(1).id, 1);
}

TEST(Parser, ExtraWhitespace){
 auto const particles = parse("  1 2 50 50 3 4\n"
                               "5 1 20 30 -1 -2   \n"
                               "   \n"
                               "   \n"
                               "6 7 80 90 0.1 -0.1\n");

  ASSERT_EQ(particles.size(), 3U);

  EXPECT_DOUBLE_EQ(particles.at(0).mass, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(0).radius, 2.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.getx(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.gety(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.getx(), 3.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.gety(), 4.0);
  EXPECT_DOUBLE_EQ(particles.at(0).id, 0);

  EXPECT_DOUBLE_EQ(particles.at(1).mass, 5.0);
  EXPECT_DOUBLE_EQ(particles.at(1).radius, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.getx(), 20.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.gety(), 30.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.getx(), -1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.gety(), -2.0);
  EXPECT_DOUBLE_EQ(particles.at(1).id, 1);

  EXPECT_DOUBLE_EQ(particles.at(2).mass, 6.0);
  EXPECT_DOUBLE_EQ(particles.at(2).radius, 7.0);
  EXPECT_DOUBLE_EQ(particles.at(2).pos.getx(), 80.0);
  EXPECT_DOUBLE_EQ(particles.at(2).pos.gety(), 90.0);
  EXPECT_DOUBLE_EQ(particles.at(2).vel.getx(), 0.1);
  EXPECT_DOUBLE_EQ(particles.at(2).vel.gety(), -0.1);
  EXPECT_DOUBLE_EQ(particles.at(2).id, 4);
 }
  TEST(Parser, ExtraTokens){
  auto const particles = parse("1 2 50 50 3 4\n"
                                "5 1 20 30 -1 -2 6 7\n"

                                "5 1 20 30 -1 -2\n"  
                               
                               );
  ASSERT_EQ(particles.size(), 2U);
  EXPECT_DOUBLE_EQ(particles.at(0).mass, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(0).radius, 2.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.getx(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).pos.gety(), 50.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.getx(), 3.0);
  EXPECT_DOUBLE_EQ(particles.at(0).vel.gety(), 4.0);
  EXPECT_DOUBLE_EQ(particles.at(0).id, 0);

  EXPECT_DOUBLE_EQ(particles.at(1).mass, 5.0);
  EXPECT_DOUBLE_EQ(particles.at(1).radius, 1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.getx(), 20.0);
  EXPECT_DOUBLE_EQ(particles.at(1).pos.gety(), 30.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.getx(), -1.0);
  EXPECT_DOUBLE_EQ(particles.at(1).vel.gety(), -2.0);
  EXPECT_DOUBLE_EQ(particles.at(1).id, 2);
}
  

