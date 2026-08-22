import { Injectable } from '@angular/core';
import { Subject, Observable } from 'rxjs';

@Injectable({
	providedIn: 'root'
})
export class ForceRefreshService {
	private requests = new Subject<number>();

	get requests$(): Observable<number> {
		return this.requests.asObservable();
	}

	request(targets: number): void {
		this.requests.next(targets);
	}
}
