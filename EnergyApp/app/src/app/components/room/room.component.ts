import { Component, OnInit } from '@angular/core';
import { CommonModule } from '@angular/common';
import { NavbarComponent } from '../navbar/navbar.component';
import { SidebarComponent } from '../sidebar/sidebar.component';
import { RoomService } from 'src/app/services/room.service';
import { SimpleInfo } from 'src/app/models/SimpleInfo';
import { MeterData } from 'src/app/models/MeterData';
import { DashboardComponent } from '../dashboard/dashboard.component';
import { ActivatedRoute, Router } from '@angular/router';

@Component({
  selector: 'app-room',
  standalone: true,
  imports: [CommonModule,NavbarComponent,SidebarComponent,DashboardComponent],
  templateUrl: './room.component.html',
  styleUrls: ['./room.component.css']
})
export class RoomComponent implements OnInit
{
  constructor(public roomService : RoomService, private route : ActivatedRoute, private router: Router) {}

  CurrentRoomId : number = 1;
  Room : MeterData = {Id : 0, Name : "", RealTime : 0, Accumulated : 0, Note : ""}
  Meters : MeterData[] = [];

  Building : SimpleInfo = {Id : 0, Name : ""}
  RoomList : SimpleInfo[] = [];

  ngOnInit(): void 
  {
    // this.route.params.subscribe(params => 
    // {
    //   this.CurrentRoomId = parseFloat(params['id']);
    //   this.roomService.getMeterData(params['id']).subscribe(x => this.Room = x);
    //   this.roomService.getSubMeterData(params['id']).subscribe(x => this.Meters = x);
    // });

    // this.roomService.getParentSimpleInfo().subscribe(x => this.Building = x);
    // this.roomService.getSiblingsSimpleInfo().subscribe(x => this.RoomList = x);

    this.route.params.subscribe(params => 
    {
      this.roomService.GetRoomData(params['id']);
      
      this.roomService.SiblingSimpleInfo.subscribe(x => this.RoomList = x);
      this.roomService.SubMeterData.subscribe(x => this.Meters = x);
      this.roomService.ParentSimpleInfo.subscribe(x => this.Building = x);
      this.roomService.MeterData.subscribe(x => this.Room = x);
    });
  }

  SideBarClick(id:number)
  {
    console.log(this.RoomList);
    // this.router.navigate(['room', id]);
  }
}
