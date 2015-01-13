// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#include "ConstituencyLabelerPipe.h"
#include "ConstituencyLabelerFeatures.h"
#include "ConstituencyLabelerPart.h"
#include "ConstituencyLabelerFeatureTemplates.h"

void ConstituencyLabelerFeatures::AddNodeFeatures(
    ConstituencyLabelerInstanceNumeric *sentence,
    int position) {
  // Add an empty feature vector.
  CHECK(!input_features_nodes_[position]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_nodes_[position] = features;

  const ParseTreeNumeric &parse_tree = sentence->GetParseTree();
  const std::vector<ParseTreeNumericNode*> &non_terminals =
    parse_tree.non_terminals();

  // Only 4 bits are allowed in feature_type.
  uint8_t feature_type = ConstituencyLabelerFeatureTemplateParts::NODE;
  CHECK_LT(feature_type, 16);
  CHECK_GE(feature_type, 0);

  ParseTreeNumericNode *node = non_terminals[position];
  ParseTreeNumericNode *parent_node = non_terminals[position]->parent();
  ParseTreeNumericNode *left_node = NULL;
  ParseTreeNumericNode *right_node = NULL;
  int current_index = -1;
  if (parent_node) {
    for (int i = 0; i < parent_node->children().size(); ++i) {
      ParseTreeNumericNode *current = parent_node->GetChild(i);
      if (current == node) {
        current_index = i;
      } else {
        if (current_index < 0) {
          left_node = current;
        } else {
          right_node = current;
        }
      }
    }
  } else {
    current_index = 0;
  }
  CHECK_GE(current_index, 0);

  // Parent node.
  uint16_t PCID = (parent_node)? parent_node->label() : TOKEN_START;
  // Current node.
  uint16_t CID = node->label();
  // Left sibling node.
  uint16_t pCID = (left_node)? left_node->label() : TOKEN_START;
  // Right sibling node.
  uint16_t nCID = (right_node)? right_node->label() : TOKEN_STOP;
  // Current word, if pre-terminal.
  uint16_t WID = (node->IsPreTerminal())? sentence->GetFormId(node->start()) :
    TOKEN_STOP;
  // Previous word, if previous node is pre-terminal.
  uint16_t pWID = (left_node && left_node->IsPreTerminal())?
    sentence->GetFormId(left_node->start()) : TOKEN_START;
  // Next word, if next node is pre-terminal.
  uint16_t nWID = (right_node && right_node->IsPreTerminal())?
    sentence->GetFormId(right_node->start()) : TOKEN_STOP;
  // Child index.
  uint8_t current_index_code = (current_index < 0xff)? current_index : 0xff;
  // Preterminal code.
  uint8_t preterminal_code = 0x0;
  if (node->IsPreTerminal()) {
    preterminal_code = 0x1;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(ConstituencyLabelerFeatureTemplateNode::COUNT, 256);

  uint64_t fkey;

  // Code for feature type.
  uint8_t flags = feature_type; // 4 bits.
  flags |= (preterminal_code << 4); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(ConstituencyLabelerFeatureTemplateNode::BIAS, flags);
  AddFeature(fkey, features);

  // Other features.
  fkey = encoder_.CreateFKey_W(ConstituencyLabelerFeatureTemplateNode::CID, flags, CID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_pCID, flags, CID, pCID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_nCID, flags, CID, nCID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_PCID, flags, CID, PCID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(ConstituencyLabelerFeatureTemplateNode::CID_CHIDX, flags, CID, current_index_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WWP(ConstituencyLabelerFeatureTemplateNode::CID_PCID_CHIDX, flags, CID, PCID, current_index_code);
  AddFeature(fkey, features);

  if (node->IsPreTerminal()) {
    fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_WID, flags, CID, WID);
    AddFeature(fkey, features);
  }

  if (left_node && left_node->IsPreTerminal()) {
    fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_pWID, flags, CID, pWID);
    AddFeature(fkey, features);
  }

  if (right_node && right_node->IsPreTerminal()) {
    fkey = encoder_.CreateFKey_WW(ConstituencyLabelerFeatureTemplateNode::CID_nWID, flags, CID, nWID);
    AddFeature(fkey, features);
  }
}
