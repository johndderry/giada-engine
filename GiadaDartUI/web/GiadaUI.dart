// Copyright (c) 2015, <your name>. All rights reserved. Use of this source code
// is governed by a BSD-style license that can be found in the LICENSE file.

import 'dart:html';

ButtonElement rewind, play, recAction, recSample, metronome,
  incrBPM, decrBPM, meter, multiplier, divider, suspend, advance,
  songMultiplier, songDivider;

TextInputElement  bpm;
SelectElement     quantizer;
Meter             songMeter, beatMeter; 

void main() {
  
  rewind = querySelector("#rewind");
  play = querySelector("#play");
  recAction = querySelector("#recAction");
  recSample = querySelector("#recSample");
  metronome = querySelector("#metronome");
  incrBPM = querySelector("#incrBpm");
  decrBPM = querySelector("#decrBpm");
  meter = querySelector("#meter");
  multiplier = querySelector("#multiplier");
  divider = querySelector("#divider");
  suspend = querySelector("#suspend");
  advance = querySelector("#advance");
  songMultiplier = querySelector("#songMultiplier");
  songDivider = querySelector("#songDivider");

  bpm = querySelector("#bpm");
  quantizer = querySelector("#quantizer");
  
  songMeter = new Meter(querySelector("#songMeter"), 16);
  beatMeter = new Meter(querySelector("#beatMeter"), 16);

  advance.onClick.listen( _advance );
}

void _advance(Event e) {
  
  songMeter.advance();
  
}

class Meter {
  
  TableElement            meterElement;
  List<TableCellElement>  cellList = new List();        
  int count, position = 0;
  
  Meter( this.meterElement, this.count ) {
    create();
  }

  void create() {
    TableRowElement row = new TableRowElement();
    meterElement.append( row );

    TableCellElement cell = new TableCellElement();
    cell.append(new ImageElement(src:"images/black.png", width:20, height:20));
    row.append( cell );
    cellList.add( cell );

    for( int i = 1; i < count; i++ ) {
      cell = new TableCellElement();
      cell.append(new ImageElement(src:"images/white.png", width:20, height:20));
      row.append( cell );
      cellList.add( cell );
    }
    
  }
  
  void advance() {
    cellList[position].lastChild.remove();
    cellList[position]..append(new ImageElement(src:"images/white.png", width:20, height:20));

    if( ++position >= count ) position = 0;

    cellList[position].lastChild.remove();
    cellList[position].append(new ImageElement(src:"images/black.png", width:20, height:20));

  }
}

